
from collections.abc import Mapping, Sequence
from dataclasses import dataclass
import struct

import compare_exe


@dataclass
class Api:
    name: str
    name_rva: int
    hint: int
    ordinal: int | None


@dataclass
class Dll:
    name: str
    descriptor: int
    lookup: int
    name_rva: int
    iat: int
    functions: list[Api]


def offset(image: compare_exe.PE, rva: int) -> int:
    result = image.offset(image.base + rva)
    if result is None:
        raise ValueError(f'unmapped import metadata at RVA {rva:#x}')
    return result


def imports(image: compare_exe.PE) -> list[Dll]:
    rva = image.directory(1)[0]
    result: list[Dll] = []
    while True:
        lookup, _, _, name_rva, iat = struct.unpack_from('<IIIII', image.data, offset(image, rva))
        if not name_rva:
            return result
        functions: list[Api] = []
        cursor = lookup or iat
        while value := struct.unpack_from('<I', image.data, offset(image, cursor))[0]:
            if value & 0x80000000:
                functions.append(Api(f'#{value & 0xffff}', 0, 0, value & 0xffff))
            else:
                hint = struct.unpack_from('<H', image.data, offset(image, value))[0]
                functions.append(Api(image.cstr(image.base + value + 2), value, hint, None))
            cursor += 4
        result.append(Dll(image.cstr(image.base + name_rva), rva, lookup, name_rva, iat, functions))
        rva += 20


def restore(image: compare_exe.PE, reference: compare_exe.PE,
            fixups: Sequence[tuple[str, int, str, int]],
            symbols: Mapping[str, tuple[int, str]]) -> tuple[bytes, dict[str, int]]:

    current = {dll.name.lower(): dll for dll in imports(image)}
    wanted = imports(reference)
    if current.keys() != {dll.name.lower() for dll in wanted}:
        raise ValueError('the import DLL set differs from the original')
    if image.directory(1) != reference.directory(1) or image.directory(12) != reference.directory(12):
        raise ValueError('the import directories differ from the original layout')
    redirects: dict[int, int] = {}
    for dll in wanted:
        old = current[dll.name.lower()]
        old_order = {api.name: i for i, api in enumerate(old.functions)}
        if old_order.keys() != {api.name for api in dll.functions}:
            raise ValueError(f'the imported functions differ for {dll.name}')
        for i, api in enumerate(dll.functions):
            redirects[image.base + old.iat + old_order[api.name] * 4] = image.base + dll.iat + i * 4

    data = bytearray(image.data)
    metadata = image.directory(1)[0]
    rdata_address, rdata_size, _, _ = reference.sections['.rdata']
    size = rdata_address + rdata_size - image.base - metadata
    start = offset(image, metadata)
    data[start:start + size] = bytes(size)
    iat, iat_size = image.directory(12)
    start = offset(image, iat)
    data[start:start + iat_size] = bytes(iat_size)

    def store(rva: int, value: bytes) -> None:
        start = offset(image, rva)
        if len(image.read(image.base + rva, len(value))) != len(value):
            raise ValueError('import metadata exceeds its initialized section')
        data[start:start + len(value)] = value

    for dll in wanted:
        store(dll.descriptor, struct.pack('<IIIII', dll.lookup, 0, 0, dll.name_rva, dll.iat))
        store(dll.name_rva, dll.name.encode('latin-1') + b'\0')
        for i, api in enumerate(dll.functions):
            value = 0x80000000 | api.ordinal if api.ordinal is not None else api.name_rva
            entry = struct.pack('<I', value)
            store(dll.iat + i * 4, entry)
            if dll.lookup:
                store(dll.lookup + i * 4, entry)
            if api.ordinal is None:
                store(api.name_rva, struct.pack('<H', api.hint) + api.name.encode('latin-1') + b'\0')




    for anchor, displacement, target, addend in fixups:
        address = symbols[anchor][0] + displacement
        old_target = symbols[target][0]
        new_target = redirects[old_target]
        at = offset(image, address - image.base)
        actual = struct.unpack_from('<I', image.data, at)[0]
        if actual != (old_target + addend) & 0xffffffff:
            raise ValueError(f'import relocation disagrees with the link map: {target}')
        struct.pack_into('<I', data, at, (new_target + addend) & 0xffffffff)


    for address, _ in symbols.values():
        if image.section_of(address) not in ('.text', '.extra'):
            continue
        body = image.read(address, 6)
        if body[:2] == b'\xff\x25' and len(body) == 6:
            target = struct.unpack_from('<I', body, 2)[0]
            if target in redirects:
                struct.pack_into('<I', data, offset(image, address + 2 - image.base), redirects[target])
    changed = {name: redirects[address] for name, (address, _) in symbols.items()
               if name.startswith('__imp_') and address in redirects}
    return bytes(data), changed
