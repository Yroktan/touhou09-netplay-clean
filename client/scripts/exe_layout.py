






from __future__ import annotations

from collections import defaultdict
from dataclasses import dataclass
from itertools import permutations
from pathlib import Path
import struct
import sys
import csv
import json
import re
from collections.abc import Collection, Mapping, Sequence

sys.path.insert(0, str(Path(__file__).resolve().parent))
from coff_archive import members
from coff_sections import split_storage
from pe_imports import restore as restore_imports
from table_audit import check_tables

import coff
import project
import compare_exe


@dataclass
class Unit:
    path: str
    obj: coff.Object


_import_fixups: list[tuple[str, int, str, int]] = []
_table_object: Path | None = None

def align(value: int, boundary: int) -> int:
    return (value + boundary - 1) & -boundary


def same_code_entry(first: coff.Section, first_address: int,
                    second: coff.Section, second_address: int) -> bool:





    return (first_address == second_address and first.data == second.data
            and not first.relocs and not second.relocs)


def same_code_pattern(section: coff.Section, original: bytes) -> bool:

    if len(section.data) != len(original):
        return False
    mask = bytearray(len(section.data))
    for offset, _, _ in section.relocs:
        mask[offset:offset + 4] = b'\1' * 4
    return all(mark or value == old for value, old, mark in zip(section.data, original, mask))


def storage_group_locations(
    units: Sequence[Unit],
    groups: Sequence[Sequence[tuple[tuple[int, int], int]]],
    votes: Mapping[tuple[int, int], Mapping[int, int]],
    reference: compare_exe.PE,
    base: int,
) -> dict[tuple[int, int], int]:






    result: dict[tuple[int, int], int] = {}
    for group in groups:
        starts = {location - offset for key, offset in group for location in votes.get(key, {})}
        sections = [units[u].obj.sections[s] for (u, s), _ in group]


        if sections and not any(section.relocs for section in sections):
            data = b''.join(section.data for section in sections)
            address, size, _, _ = reference.sections[sections[0].name]
            original = reference.read(address, size)
            found = original.find(data) if len(data) >= 8 else -1
            if found >= 0 and original.find(data, found + 1) < 0:
                starts.add(address + found - base)
        valid: list[int] = []
        for start in starts:
            for (u, s), offset in group:
                section = units[u].obj.sections[s]
                address = base + start + offset
                if (reference.section_of(address) != section.name
                        or not same_code_pattern(section, reference.read(address, len(section.data)))):
                    break
            else:
                valid.append(start)
        if len(valid) == 1:
            result.update((key, valid[0] + offset) for key, offset in group)
    return result


def fold_identical_sections(units: Sequence[Unit], owners: Mapping[str, tuple[int, int]],
                            weak: Mapping[str, str],
                            positions: dict[tuple[int, int], tuple[int, int]],
                            candidates: Collection[tuple[int, int]],
                            foldable: Collection[tuple[int, int]]) -> dict[tuple[int, int], tuple[int, int]]:






    aliases: dict[tuple[int, int], tuple[int, int]] = {}
    while True:
        count = len(aliases)
        signatures: dict[tuple[bool, bytes, tuple[tuple[object, ...], ...]], tuple[int, int]] = {}
        for key in sorted(set(candidates) - aliases.keys(),
                          key=lambda key: (key in foldable, positions[key], key)):
            u, s = key
            section = units[u].obj.sections[s]
            references: list[tuple[object, ...]] = []
            for offset, index, kind in section.relocs:
                target = units[u].obj.symbols[index]
                target_u = u
                if target.storage == 2 or target.section == 0:
                    owner = owners.get(weak.get(target.name, target.name))
                    if owner is not None:
                        target_u, target_index = owner
                        target = units[target_u].obj.symbols[target_index]
                other = (target_u, target.section - 1)
                if other == key:
                    identity: object = ('self', target.value)
                elif other in positions:
                    identity = (positions[other], target.value)
                elif target.section == -1:
                    identity = ('absolute', target.value)
                elif target.section > 0:
                    identity = ('defined', target_u, target.section, target.value)
                else:
                    identity = ('external', weak.get(target.name, target.name))
                references.append((offset, kind, identity))
            signature = (bool(section.flags & coff.CODE), section.data, tuple(references))
            previous = signatures.get(signature)
            if previous is not None and key in foldable:
                aliases[key] = previous
                positions[key] = positions[previous]
            else:
                signatures.setdefault(signature, key)
        for key in aliases:
            previous = aliases[key]
            while previous in aliases:
                previous = aliases[previous]
            aliases[key] = previous
            positions[key] = positions[previous]
        if len(aliases) == count:
            fold_reference_cycles(units, owners, weak, positions, candidates, foldable, aliases)
            if len(aliases) == count:
                return aliases


def section_target(units: Sequence[Unit], owners: Mapping[str, tuple[int, int]],
                   weak: Mapping[str, str], u: int, index: int) -> tuple[object, int]:

    target = units[u].obj.symbols[index]
    target_u = u
    if target.storage == 2 or target.section == 0:
        owner = owners.get(weak.get(target.name, target.name))
        if owner is not None:
            target_u, target_index = owner
            target = units[target_u].obj.symbols[target_index]
    if target.section > 0:
        return (target_u, target.section - 1), target.value
    if target.section == -1:
        return ('absolute',), target.value
    return ('external', weak.get(target.name, target.name)), 0


def fold_reference_cycles(units: Sequence[Unit], owners: Mapping[str, tuple[int, int]],
                          weak: Mapping[str, str],
                          positions: dict[tuple[int, int], tuple[int, int]],
                          candidates: Collection[tuple[int, int]],
                          foldable: Collection[tuple[int, int]],
                          aliases: dict[tuple[int, int], tuple[int, int]]) -> None:








    candidates = set(candidates)

    def resolve(key: tuple[int, int]) -> tuple[int, int]:
        while key in aliases:
            key = aliases[key]
        return key

    def unify(copy: tuple[int, int], original: tuple[int, int],
              assumed: dict[tuple[int, int], tuple[int, int]]) -> bool:
        copy, original = resolve(copy), resolve(original)
        if copy == original or assumed.get(copy) == original:
            return True
        if copy in assumed or copy not in foldable or copy not in candidates:
            return False
        if original not in positions or positions[original][0] == 2:
            return False
        first = units[copy[0]].obj.sections[copy[1]]
        second = units[original[0]].obj.sections[original[1]]
        if (first.flags & coff.CODE != second.flags & coff.CODE or first.data != second.data
                or [r[::2] for r in first.relocs] != [r[::2] for r in second.relocs]):
            return False
        assumed[copy] = original
        for (_, index, _), (_, other_index, _) in zip(first.relocs, second.relocs):
            target, value = section_target(units, owners, weak, copy[0], index)
            other, other_value = section_target(units, owners, weak, original[0], other_index)
            if value != other_value:
                return False
            if target == other:
                continue
            if len(target) != 2 or len(other) != 2 or not isinstance(target[0], int):
                return False
            if not unify(target, other, assumed):
                return False
        return True

    anchored: dict[tuple[bool, bytes], list[tuple[int, int]]] = defaultdict(list)
    for key in sorted(candidates - set(foldable)):
        if key in positions and positions[key][0] != 2:
            section = units[key[0]].obj.sections[key[1]]
            anchored[bool(section.flags & coff.CODE), section.data].append(key)
    for key in sorted(set(foldable) & candidates - aliases.keys()):
        section = units[key[0]].obj.sections[key[1]]
        if key in aliases or not section.relocs:
            continue
        matches = []
        for original in anchored[bool(section.flags & coff.CODE), section.data]:
            assumed: dict[tuple[int, int], tuple[int, int]] = {}
            if unify(key, original, assumed):
                matches.append(assumed)
        if matches:
            first = min(matches, key=lambda assumed: positions[assumed[key]])
            for copy, original in first.items():
                aliases[copy] = original
                positions[copy] = positions[original]


def library_symbol(label: str, available: Collection[str]) -> str | None:





    return next((candidate for candidate in
                 (label, '_' + label, '_' + label.lstrip('_'), '__' + label.lstrip('_'))
                 if candidate in available), None)


def trim_storage_padding(obj: coff.Object, extents: Mapping[str, int]) -> None:






    for symbol in obj.externals():
        size = extents.get(symbol.name)
        if size is None or symbol.section <= 0 or symbol.value:
            continue
        section = obj.sections[symbol.section - 1]
        if section.name not in ('.bss', '.data') or len(section.data) <= size:
            continue
        if (any(section.data[size:]) or any(offset + 4 > size for offset, _, _ in section.relocs)
                or any(other and other.section == symbol.section and other.value >= size
                       for other in obj.symbols)):
            raise ValueError(f'non-padding storage beyond the known extent of {symbol.name}')
        section.data = section.data[:size]


def import_thunks(units: Sequence[Unit], known: dict[str, int],
                  reference: compare_exe.PE, output_dir: Path) -> Unit | None:






    defined = {sym.name for unit in units for sym in unit.obj.externals() if sym.section > 0}
    referenced = {sym.name for unit in units for sym in unit.obj.externals()
                  if sym.section == 0 and sym.value == 0}
    text_base, text_size, _, _ = reference.sections['.text']
    original = reference.read(text_base, text_size)
    sections: list[tuple[str, int, bytes, list[tuple[int, str, int]]]] = []
    definitions: dict[str, tuple[int, int]] = {}
    imports: list[str] = []
    for symbol in sorted(referenced - defined):
        target = '__imp_' + symbol
        if target not in known:
            continue
        fingerprint = b'\xff\x25' + struct.pack('<I', known[target])
        offset = original.find(fingerprint)
        if offset < 0 or original.find(fingerprint, offset + 1) >= 0:
            continue
        address = text_base + offset
        if symbol in known and known[symbol] != address:
            raise ValueError(f'import stub address disagrees with symbols.csv: {symbol}')



        alias = '__layout_import_stub_' + symbol
        known[alias] = address


        sections.append(('.text', (coff.TEXT & ~0xf00000) | 0x100000,
                         b'\xff\x25\0\0\0\0', [(2, target, coff.DIR32)]))
        definitions[alias] = (len(sections), 0)
        imports.append(target)
        for unit in units:
            for entry in unit.obj.externals():
                if entry.name == symbol and entry.section == 0 and entry.value == 0:
                    entry.name = alias
    if not sections:
        return None
    output = output_dir / 'import_thunks.obj'
    coff.write_object(str(output), sections, definitions, imports, {})
    print('import stubs placed', len(sections))
    return Unit(str(output), coff.Object(str(output)))


def exe_flags(features: Sequence[str]) -> list[str]:

    if 'orig_checksum' not in features:
        return []
    original = Path(project.ORIGINAL).read_bytes()
    checksum = sum(value[0] for value in struct.iter_unpack('<I', original[:(len(original) // 4 - 1) * 4])) & 0xffffffff
    return [f'/DTH09_ORIGINAL_EXE_SIZE={len(original)}U',
            f'/DTH09_ORIGINAL_EXE_CHECKSUM=0x{checksum:08x}U']


def exe_objects(objects: Sequence[str], features: Sequence[str]) -> list[str]:

    reference = compare_exe.PE(project.ORIGINAL)
    text_base, text_size, _, _ = reference.sections['.text']
    text_end = text_base + text_size
    symbol_rows = compare_exe.read_symbols()
    known = {symbol: address for address, _, _, symbols in symbol_rows for symbol in symbols}
    extents = {symbol: size for address, size, _, symbols in symbol_rows
               if size and reference.section_of(address) == '.data' for symbol in symbols}
    paths = list(objects)
    global _table_object
    _table_object = next(Path(path) for path in paths if Path(path).stem == 'exe_tables')
    output_dir = Path(paths[0]).parent.with_name(Path(paths[0]).parent.name + '_layout')
    output_dir.mkdir(parents=True, exist_ok=True)
    units = [Unit(path, coff.Object(path)) for path in paths]
    needed = json.loads(Path('config/exe-libraries.json').read_text())
    extracted = output_dir / 'libraries'
    extracted.mkdir(exist_ok=True)
    seen_libraries: set[str] = set()
    for folder in project.config['build']['lib']:
        local = Path(project.ROOT) / '.wine/drive_c' / folder[3:].replace('\\', '/')
        if not local.exists():
            local = Path(folder)
        if not local.exists():
            continue
        for library in sorted(local.glob('*'), key=lambda item: item.name.lower()):
            stem = library.stem.lower()
            if stem not in needed or stem in seen_libraries or library.suffix.lower() != '.lib':
                continue
            seen_libraries.add(stem)
            for member, raw in members(library):
                if member.lower() not in needed[stem] or raw[:2] != b'\x4c\x01':
                    continue
                output = extracted / (stem + '_' + member)
                output.write_bytes(raw)
                obj = coff.Object(str(output))
                if any(section.name.startswith('.idata') for section in obj.sections):
                    continue
                units.append(Unit(str(output), obj))
    missing = needed.keys() - seen_libraries
    if missing:
        raise FileNotFoundError('missing layout libraries: ' + ', '.join(sorted(missing)))
    print('source/library objects', len(paths), len(units) - len(paths))
    thunks = import_thunks(units, known, reference, output_dir)
    if thunks is not None:
        units.append(thunks)
    storage_groups: list[list[tuple[tuple[int, int], int]]] = []
    for u, unit in enumerate(units):
        old_sections = list(unit.obj.sections)
        parts = split_storage(unit.obj)
        if u >= len(paths):
            storage_groups.extend([((u, index - 1), offset) for offset, index in group]
                                  for old, group in parts.items()
                                  if old_sections[old].name in ('.rdata', '.data')
                                  and not old_sections[old].flags & 0x1000)
        trim_storage_padding(unit.obj, extents)
    owners: dict[str, tuple[int, int]] = {}
    for u, unit in enumerate(units):
        for i, sym in enumerate(unit.obj.symbols):
            if sym and sym.storage == 2 and (sym.section > 0 or sym.section == 0 and sym.value):
                previous = owners.get(sym.name)
                old = units[previous[0]].obj.symbols[previous[1]] if previous else None


                if old is None or old.section == 0 and (sym.section > 0 or sym.value > old.value):
                    owners[sym.name] = (u, i)



    for u, i in owners.values():
        symbol = units[u].obj.symbols[i]
        if symbol.section == 0 and symbol.value:
            size = symbol.value
            units[u].obj.sections.append(coff.Section('.bss', bytes(size), [], 0xc0300080))
            symbol.section = len(units[u].obj.sections)
            symbol.value = 0

    with Path('config/symbols.csv').open() as stream:
        for row in csv.DictReader(stream):
            address = int(row['address'], 16)
            if not text_base <= address < text_end:
                continue
            candidate = library_symbol(row['name'], owners.keys())
            if candidate is not None:
                known.setdefault(candidate, address)

    weak: dict[str, str] = {}
    for unit in units:
        raw = Path(unit.path).read_bytes()
        pointer = struct.unpack_from('<I', raw, 8)[0]
        for i, sym in enumerate(unit.obj.symbols):
            if sym and sym.storage == 105:
                index = struct.unpack_from('<I', raw, pointer + (i + 1) * 18)[0]
                weak[sym.name] = unit.obj.symbols[index].name

    def name(u: int, i: int) -> str:
        sym = units[u].obj.symbols[i]
        if sym.name in weak:
            return weak[sym.name]
        if sym.name.startswith(('__real@', '??_C@')):
            owner_u, owner_i = owners.get(sym.name, (u, i))
            return f'__layout_const_{owner_u}_{owner_i}'
        if sym.storage == 2 or sym.section == 0:
            return sym.name
        return f'__layout_{u}_{i}_{sym.name}'

    chosen: set[tuple[int, int]] = set()
    for u, unit in enumerate(units):
        section_defs: dict[int, list[tuple[int, coff.Symbol]]] = defaultdict(list)
        for i, sym in enumerate(unit.obj.symbols):
            if sym and sym.storage == 2 and sym.section > 0:
                section_defs[sym.section - 1].append((i, sym))
        for s, section in enumerate(unit.obj.sections):
            if section.name.startswith('.debug'):
                continue
            definitions = section_defs[s]
            if section.flags & 0x1000 and definitions and not any(owners[sym.name] == (u, i) for i, sym in definitions):
                continue
            chosen.add((u, s))

    placed: dict[tuple[int, int], int] = {}
    candidates: dict[int, list[tuple[int, int, int]]] = defaultdict(list)
    for u, s in chosen:
        section = units[u].obj.sections[s]
        if not section.flags & coff.CODE:
            continue
        for i, sym in enumerate(units[u].obj.symbols):
            if sym and sym.section == s + 1 and sym.name in known:
                if sym.storage == 2 and owners.get(sym.name) != (u, i):
                    continue
                candidates[known[sym.name] - sym.value].append((u, s, i))
    addresses = sorted(a for a in candidates if text_base <= a < text_end)
    slots = {a: b - a for a, b in zip(addresses, addresses[1:] + [text_end])}
    for address in addresses:

        u, s, _ = min(candidates[address])
        if (u, s) not in placed:
            placed[u, s] = address - text_base

    image = coff.Image(project.ORIGINAL)
    original = image.read(text_base, text_size)

    def calls_match(u: int, s: int, offset: int) -> bool:






        section = units[u].obj.sections[s]
        for off, index, kind in section.relocs:
            if kind not in (coff.DIR32, coff.REL32):
                continue
            target = units[u].obj.symbols[index]
            addend = int.from_bytes(section.data[off:off + 4], 'little', signed=True)
            value = int.from_bytes(original[offset + off:offset + off + 4], 'little',
                                   signed=kind == coff.REL32)
            if kind == coff.REL32:
                value += text_base + offset + off + 4
            known_target = known.get(weak.get(target.name, target.name))
            if known_target is not None and value != (known_target + addend) & 0xffffffff:
                return False
            if kind != coff.REL32:
                continue
            target_u = u
            if target.storage == 2 or target.section == 0:
                owner = owners.get(weak.get(target.name, target.name))
                if owner is None:
                    continue
                target_u, target_index = owner
                target = units[target_u].obj.symbols[target_index]
            if target.section <= 0:
                continue
            body = units[target_u].obj.sections[target.section - 1]
            if not body.flags & coff.CODE:
                continue
            relative = int.from_bytes(original[offset + off:offset + off + 4], 'little', signed=True)
            start = offset + off + 4 + relative - target.value - addend
            if start < 0 or not same_code_pattern(body, original[start:start + len(body.data)]):
                return False
        return True

    inferred = 0
    while True:
        previous_count = inferred
        for u, section_index in sorted(chosen):
            section = units[u].obj.sections[section_index]
            if not section.flags & coff.CODE or (u, section_index) in placed or len(section.data) < 8:
                continue
            mask = bytearray(len(section.data))
            for offset, _, _ in section.relocs:
                mask[offset:offset + 4] = b'\1' * 4
            spans: list[tuple[int, int]] = []
            start = 0
            while start < len(mask):
                if mask[start]:
                    start += 1
                    continue
                end = start + 1
                while end < len(mask) and not mask[end]:
                    end += 1
                spans.append((start, end))
                start = end
            if not spans:
                continue
            first, last = max(spans, key=lambda span: span[1] - span[0])
            needle = section.data[first:last]
            if len(needle) < 6:


                anchors = [(off, known.get(weak.get(units[u].obj.symbols[index].name,
                                                   units[u].obj.symbols[index].name)))
                           for off, index, kind in section.relocs if kind == coff.DIR32]
                anchor = next(((off, address) for off, address in anchors if address is not None), None)
                if anchor is None:


                    located_call = False
                    for _, index, kind in section.relocs:
                        if kind != coff.REL32:
                            continue
                        target = units[u].obj.symbols[index]
                        target_u = u
                        if target.storage == coff.CLASS_EXTERNAL or target.section == 0:
                            owner = owners.get(weak.get(target.name, target.name))
                            if owner is not None:
                                target_u, index = owner
                                target = units[target_u].obj.symbols[index]
                        if target.name in known or (target_u, target.section - 1) in placed:
                            located_call = True
                            break
                    if not located_call:
                        continue
                else:
                    first, address = anchor
                    addend = int.from_bytes(section.data[first:first + 4], 'little', signed=True)
                    needle = struct.pack('<I', (address + addend) & 0xffffffff)
            hits: list[int] = []
            cursor = 0
            while True:
                found = original.find(needle, cursor)
                if found < 0:
                    break
                offset = found - first
                if (offset >= 0 and offset + len(mask) <= len(original)
                        and all(m or a == b for a, b, m in zip(section.data, original[offset:offset + len(mask)], mask))
                        and calls_match(u, section_index, offset)):
                    hits.append(offset)
                cursor = found + 1
            if len(hits) == 1:
                start = hits[0]
                end = start + len(section.data)
                if any(start < offset + len(units[key[0]].obj.sections[key[1]].data) and offset < end
                       and not same_code_entry(section, start, units[key[0]].obj.sections[key[1]], offset)
                       for key, offset in placed.items()):
                    continue
                placed[u, section_index] = hits[0]
                inferred += 1
        if inferred == previous_count:
            break



    located_data: dict[tuple[int, int], int] = {}
    for u, section_index in sorted(chosen):
        section = units[u].obj.sections[section_index]
        if section.name not in ('.data', '.rdata'):
            continue
        for symbol in units[u].obj.symbols:
            if symbol and symbol.section == section_index + 1 and symbol.name in known:
                located_data[u, section_index] = known[symbol.name] - symbol.value - text_base
    discovered = 0
    changed = True
    while changed:
        changed = False
        for (u, s), source_offset in list(placed.items()) + list(located_data.items()):
            section = units[u].obj.sections[s]
            mask = bytearray(len(section.data))
            for off, _, _ in section.relocs:
                mask[off:off + 4] = b'\1' * 4
            try:
                source = image.read(text_base + source_offset, len(section.data))
            except ValueError:
                continue
            if len(source) != len(section.data) or not all(mark or value == old for value, old, mark in
                       zip(section.data, source, mask)):
                continue
            for off, index, kind in section.relocs:
                if kind not in (coff.DIR32, coff.REL32):
                    continue
                target = units[u].obj.symbols[index]
                target_u = u
                if target.storage == 2 or target.section == 0:
                    owner = owners.get(weak.get(target.name, target.name))
                    if owner is None:
                        continue
                    target_u, target_index = owner
                    target = units[target_u].obj.symbols[target_index]
                key = (target_u, target.section - 1)
                if key not in chosen or key in placed or key in located_data:
                    continue
                body = units[key[0]].obj.sections[key[1]]
                addend = int.from_bytes(section.data[off:off + 4], 'little', signed=kind == coff.REL32)
                address = int.from_bytes(source[off:off + 4], 'little', signed=kind == coff.REL32)
                if kind == coff.REL32:
                    address += text_base + source_offset + off + 4
                start = address - target.value - addend - text_base
                end = start + len(body.data)
                if not body.flags & coff.CODE:
                    if body.name not in ('.data', '.rdata'):
                        continue
                    try:
                        raw = image.read(text_base + start, len(body.data))
                    except ValueError:
                        continue
                    data_mask = bytearray(len(body.data))
                    for data_off, _, _ in body.relocs:
                        data_mask[data_off:data_off + 4] = b'\1' * 4
                    if len(raw) != len(body.data) or not all(mark or value == old for value, old, mark in
                                                           zip(body.data, raw, data_mask)):
                        continue
                    located_data[key] = start
                    changed = True
                    continue
                if not 0 <= start < end <= len(original):
                    continue
                if any(start < at + len(units[k[0]].obj.sections[k[1]].data) and at < end
                       and not same_code_entry(body, start, units[k[0]].obj.sections[k[1]], at)
                       for k, at in placed.items()):
                    continue
                target_mask = bytearray(len(body.data))
                for target_off, _, _ in body.relocs:
                    target_mask[target_off:target_off + 4] = b'\1' * 4
                if not all(mark or value == old for value, old, mark in
                           zip(body.data, original[start:end], target_mask)):
                    continue
                placed[key] = start
                changed = True
                discovered += 1
    print('call-placed sections', discovered)
    print('content-placed sections', inferred)
    all_starts = sorted(set(placed.values()))
    slots = {start + text_base: end - start for start, end in zip(all_starts, all_starts[1:] + [text_size])}


    roots = {key for key in chosen if not units[key[0]].obj.sections[key[1]].flags & coff.CODE and (key[0] < len(paths) and units[key[0]].obj.sections[key[1]].flags & 0x80000000 or units[key[0]].obj.sections[key[1]].name in ('.drectve', '.sxdata', '.data1') or units[key[0]].obj.sections[key[1]].name.startswith('.CRT'))}


    roots.update(key for key in chosen if key[0] >= len(paths)
                 and units[key[0]].obj.sections[key[1]].name in ('.rdata', '.data')
                 and not units[key[0]].obj.sections[key[1]].flags & 0x1000)
    for u, s in list(roots):
        section = units[u].obj.sections[s]
        if section.name == '.sxdata':
            for (index,) in struct.iter_unpack('<I', section.data):
                sym = units[u].obj.symbols[index]
                if sym.section > 0:
                    roots.add((u, sym.section - 1))
    live = roots | set(placed)
    queue = list(live)
    referenced_storage: set[tuple[int, int]] = set()
    while queue:
        u, s = queue.pop()
        for _, index, _ in units[u].obj.sections[s].relocs:
            sym = units[u].obj.symbols[index]
            target_u = u
            if sym.storage == 2 or sym.section == 0:
                owner = owners.get(weak.get(sym.name, sym.name))
                if owner is None:
                    continue
                target_u, target_index = owner
                sym = units[target_u].obj.symbols[target_index]
            if sym.section > 0:
                key = (target_u, sym.section - 1)
                referenced_storage.add(key)
                if key in chosen and key not in live:
                    live.add(key)
                    queue.append(key)

    text = bytearray(b'\xcc' * text_size)
    extra = bytearray()
    positions: dict[tuple[int, int], tuple[int, int]] = {}
    trampolines: list[tuple[int, int, int]] = []
    for u, s in sorted(live):
        section = units[u].obj.sections[s]
        if not section.flags & coff.CODE:
            continue
        offset = placed.get((u, s))
        if offset is not None and len(section.data) <= slots[offset + text_base]:
            text[offset:offset + len(section.data)] = section.data
            positions[u, s] = (1, offset)
        else:
            boundary = 1 << max(0, ((section.flags >> 20) & 15) - 1)
            start = align(len(extra), boundary)
            extra.extend(b'\xcc' * (start - len(extra)))
            extra.extend(section.data)
            positions[u, s] = (2, start)
            if offset is not None:
                text[offset:offset + 5] = b'\xe9\0\0\0\0'
                trampolines.append((offset, u, s))

    readonly = {(u, s) for u, s in live if units[u].obj.sections[s].flags & 0x40
                and not units[u].obj.sections[s].flags & (coff.CODE | 0x80000000 | 0x800)
                and units[u].obj.sections[s].name != '.drectve'}
    iat_rva, iat_size = reference.directory(12)
    rd_base = align(reference.base + iat_rva + iat_size, 8)
    rd_end = reference.base + reference.directory(1)[0]
    rd_orig = image.read(rd_base, rd_end - rd_base)
    rd_data = bytearray(len(rd_orig))
    rd_used = bytearray(len(rd_orig))
    votes: dict[tuple[int, int], dict[int, int]] = defaultdict(lambda: defaultdict(int))
    for key in readonly & located_data.keys():
        votes[key][text_base + located_data[key] - rd_base] += 1
    for (u, s), code_offset in placed.items():
        section = units[u].obj.sections[s]
        mask = bytearray(len(section.data))
        for off, _, _ in section.relocs:
            mask[off:off + 4] = b'\1' * 4
        if not all(m or a == b for a, b, m in zip(section.data, original[code_offset:code_offset + len(mask)], mask)):
            continue
        for off, index, typ in section.relocs:
            if typ != coff.DIR32:
                continue
            target = units[u].obj.symbols[index]
            owner_u = u
            if target.storage == 2 or target.section == 0:
                owner = owners.get(weak.get(target.name, target.name))
                if owner is None:
                    continue
                owner_u, target_index = owner
                target = units[owner_u].obj.symbols[target_index]
            key = (owner_u, target.section - 1)
            if key not in readonly:
                continue
            addend = int.from_bytes(section.data[off:off + 4], 'little')
            address = int.from_bytes(original[code_offset + off:code_offset + off + 4], 'little')
            votes[key][address - target.value - addend - rd_base] += 1
    rd_known: set[tuple[int, int]] = set()
    for u, s in readonly:
        for sym in units[u].obj.symbols:
            if sym and sym.section == s + 1 and sym.name in known:
                votes[u, s][known[sym.name] - sym.value - rd_base] += 100
                rd_known.add((u, s))
    for key, offset in storage_group_locations(units, storage_groups, votes, reference, rd_base).items():
        if key in readonly:
            votes[key][offset] += 1
    rd_masks: dict[tuple[int, int], bytearray] = {}
    for u, s in readonly:
        section = units[u].obj.sections[s]
        mask = bytearray(len(section.data))
        for off, _, _ in section.relocs:
            mask[off:off + 4] = b'\1' * 4
        rd_masks[u, s] = mask

    def rd_fits(key: tuple[int, int], offset: int, exact: bool) -> bool:
        section = units[key[0]].obj.sections[key[1]]
        boundary = 1 << max(0, ((section.flags >> 20) & 15) - 1)
        if key in rd_known:
            boundary = min(boundary, 4)
        if (rd_base + offset) % boundary or not 0 <= offset <= len(rd_data) - len(section.data):
            return False
        mask = rd_masks[key]
        return all((not rd_used[offset + i] or rd_used[offset + i] == 1 and not mask[i] and rd_data[offset + i] == value)
                   and (not exact or mask[i] or rd_orig[offset + i] == value)
                   for i, value in enumerate(section.data))

    def rd_place(key: tuple[int, int], offset: int) -> None:
        data = units[key[0]].obj.sections[key[1]].data
        rd_data[offset:offset + len(data)] = data
        rd_used[offset:offset + len(data)] = b'\1' * len(data)
        for i, marked in enumerate(rd_masks[key]):
            if marked:
                rd_used[offset + i] = 2
        positions[key] = (3, offset)

    for key in sorted(readonly, key=lambda k: (k not in rd_known, -max(votes[k].values(), default=0), -len(units[k[0]].obj.sections[k[1]].data), k)):
        section = units[key[0]].obj.sections[key[1]]
        options = sorted(votes[key], key=lambda offset: -votes[key][offset])
        for offset in options:
            if rd_fits(key, offset, True):
                rd_place(key, offset)
                break
        if key in positions:
            continue
        mask = rd_masks[key]
        spans: list[tuple[int, int]] = []
        start = 0
        while start < len(mask):
            if mask[start]:
                start += 1
                continue
            end = start + 1
            while end < len(mask) and not mask[end]:
                end += 1
            spans.append((start, end))
            start = end
        if spans:
            first, last = max(spans, key=lambda span: span[1] - span[0])
            needle = section.data[first:last]
            cursor = 0
            while needle:
                found = rd_orig.find(needle, cursor)
                if found < 0:
                    break
                offset = found - first
                if rd_fits(key, offset, True):
                    rd_place(key, offset)
                    break
                cursor = found + 1
    rd_exact = sum(key in positions for key in readonly)
    overflow = 0
    for key in sorted(readonly - positions.keys(), key=lambda k: -len(units[k[0]].obj.sections[k[1]].data)):
        section = units[key[0]].obj.sections[key[1]]
        boundary = 1 << max(0, ((section.flags >> 20) & 15) - 1)
        size = len(section.data)
        offset = 0
        while offset + size <= len(rd_data):
            offset = align(rd_base + offset, boundary) - rd_base
            occupied = next((i for i in range(offset, offset + size) if rd_used[i]), -1)
            if occupied < 0:
                rd_place(key, offset)
                break
            offset = occupied + 1
        if key not in positions:
            offset = align(len(extra), boundary)
            extra.extend(bytes(offset - len(extra)))
            extra.extend(section.data)
            positions[key] = (2, offset)
            overflow += size
    print('read-only data:', rd_exact, 'exact sections;', overflow, 'overflow bytes')

    writable = {(u, s) for u, s in live if units[u].obj.sections[s].name in ('.data', '.bss')
                or units[u].obj.sections[s].name.startswith('.CRT')}
    data_base, data_size, _, data_raw = reference.sections['.data']
    wd_votes: dict[tuple[int, int], dict[int, int]] = defaultdict(lambda: defaultdict(int))
    wd_known: set[tuple[int, int]] = set()
    for key in writable & located_data.keys():
        wd_votes[key][text_base + located_data[key] - data_base] += 1
    for key in writable:
        u, s = key
        for sym in units[u].obj.symbols:
            if sym and sym.section == s + 1 and sym.name in known:
                wd_votes[key][known[sym.name] - sym.value - data_base] += 10000
                wd_known.add(key)
    for (u, s), code_offset in placed.items():
        section = units[u].obj.sections[s]
        mask = bytearray(len(section.data))
        for off, _, _ in section.relocs:
            mask[off:off + 4] = b'\1' * 4
        if not all(mark or value == old for value, old, mark in
                   zip(section.data, original[code_offset:code_offset + len(mask)], mask)):
            continue
        for off, index, kind in section.relocs:
            if kind != coff.DIR32:
                continue
            target = units[u].obj.symbols[index]
            target_u = u
            if target.storage == 2 or target.section == 0:
                owner = owners.get(weak.get(target.name, target.name))
                if owner is None:
                    continue
                target_u, target_index = owner
                target = units[target_u].obj.symbols[target_index]
            key = (target_u, target.section - 1)
            if key in writable:
                addend = int.from_bytes(section.data[off:off + 4], 'little', signed=True)
                address = int.from_bytes(original[code_offset + off:code_offset + off + 4], 'little')
                wd_votes[key][address - target.value - addend - data_base] += 1
    for key, offset in storage_group_locations(units, storage_groups, wd_votes, reference, data_base).items():
        if key in writable:
            wd_votes[key][offset] += 1

    def operand_address(u: int, index: int) -> int | None:
        target = units[u].obj.symbols[index]
        symbol = weak.get(target.name, target.name)
        if symbol in known:
            return known[symbol]
        if target.storage == coff.CLASS_EXTERNAL or target.section == 0:
            owner = owners.get(symbol)
            if owner is None:
                return None
            u, index = owner
            target = units[u].obj.symbols[index]
        location = positions.get((u, target.section - 1))
        if location is None:
            return None
        section, offset = location
        bases = {1: text_base, 3: rd_base, 4: data_base}
        return bases[section] + offset + target.value if section in bases else None




    data_original = reference.read(data_base, data_raw)
    for u, s in writable:
        section = units[u].obj.sections[s]
        if section.name != '.data' or section.flags & 0x80:
            continue
        operands: list[tuple[int, bytes]] = []
        for off, index, kind in section.relocs:
            address = operand_address(u, index) if kind == coff.DIR32 else None
            if address is not None:
                addend = int.from_bytes(section.data[off:off + 4], 'little', signed=True)
                operands.append((off, struct.pack('<I', (address + addend) & 0xffffffff)))
        if not operands:
            continue
        first, needle = operands[0]
        cursor = 0
        candidates: set[int] = set()
        while (found := data_original.find(needle, cursor)) >= 0:
            start = found - first
            if (0 <= start <= len(data_original) - len(section.data)
                    and same_code_pattern(section, data_original[start:start + len(section.data)])
                    and all(data_original[start + off:start + off + 4] == value for off, value in operands)):
                candidates.add(start)
            cursor = found + 1
        if len(candidates) == 1:
            wd_votes[u, s][candidates.pop()] += 1



    data_cut = data_raw
    for key in writable:
        section = units[key[0]].obj.sections[key[1]]
        if section.flags & 0x80:
            for offset in wd_votes[key]:
                if 0 <= offset < data_cut < offset + len(section.data):
                    data_cut = offset
    wd_init = bytearray(data_cut)
    wd_bss = bytearray(data_size - data_cut)
    wd_used = bytearray(data_size)
    wd_positions: dict[tuple[int, int], int] = {}
    wd_masks: dict[tuple[int, int], bytearray] = {}
    for key in writable:
        section = units[key[0]].obj.sections[key[1]]
        mask = bytearray(len(section.data))
        for off, _, _ in section.relocs:
            mask[off:off + 4] = b'\1' * 4
        wd_masks[key] = mask

    def wd_fits(key: tuple[int, int], offset: int, exact: bool, alias: bool = False) -> bool:
        section = units[key[0]].obj.sections[key[1]]
        size = len(section.data)
        boundary = 1 << max(0, ((section.flags >> 20) & 15) - 1)
        if key in wd_known:
            boundary = min(boundary, 4)
        if offset < 0 or offset + size > len(wd_used) or (data_base + offset) % boundary:
            return False
        if offset < data_cut < offset + size:
            return False
        if not section.flags & 0x80 and offset + size > data_cut:
            return False
        old = image.read(data_base + offset, size)
        mask = wd_masks[key]
        for i, value in enumerate(section.data):
            if exact and i < len(old) and not mask[i] and value != old[i]:
                return False
            if wd_used[offset + i]:
                if not alias or mask[i] or wd_used[offset + i] == 2:
                    return False
                previous = wd_init[offset + i] if offset + i < data_cut else wd_bss[offset + i - data_cut]
                if value != previous:
                    return False
        return True

    def wd_place(key: tuple[int, int], offset: int) -> None:
        section = units[key[0]].obj.sections[key[1]]
        data = section.data
        if offset < data_cut:
            wd_init[offset:offset + len(data)] = data
            positions[key] = (4, offset)
        else:
            wd_bss[offset - data_cut:offset - data_cut + len(data)] = data
            positions[key] = (5, offset - data_cut)
        wd_positions[key] = offset
        wd_used[offset:offset + len(data)] = b'\1' * len(data)
        for i, marked in enumerate(wd_masks[key]):
            if marked:
                wd_used[offset + i] = 2

    crt = {key for key in writable if units[key[0]].obj.sections[key[1]].name.startswith('.CRT')}
    startup = reference.read(data_base, 0xc0)
    for u, s in crt:
        section = units[u].obj.sections[s]
        if len(section.data) != 4 or len(section.relocs) != 1:
            continue
        off, index, kind = section.relocs[0]
        if off != 0 or kind != coff.DIR32:
            continue
        target = units[u].obj.symbols[index]
        target_u = u
        if target.storage == coff.CLASS_EXTERNAL or target.section == 0:
            owner = owners.get(weak.get(target.name, target.name))
            if owner is None:
                continue
            target_u, target_index = owner
            target = units[target_u].obj.symbols[target_index]
        location = positions.get((target_u, target.section - 1))
        if location is None or location[0] != 1:
            continue
        address = text_base + location[1] + target.value + int.from_bytes(section.data, 'little')
        slots = [offset for offset in range(0, len(startup), 4)
                 if int.from_bytes(startup[offset:offset + 4], 'little') == address]
        if len(slots) == 1:
            wd_votes[u, s][slots[0]] += 1000
    crt_order = sorted(crt, key=lambda key: (units[key[0]].obj.sections[key[1]].name,
                       max(wd_votes[key], key=wd_votes[key].get) if wd_votes[key] else data_size, key))
    cursor = 0
    for key in crt_order:
        section = units[key[0]].obj.sections[key[1]]
        choices = sorted(wd_votes[key], key=lambda offset: -wd_votes[key][offset])
        wanted = next((offset for offset in choices if cursor <= offset < 0xc0), cursor)
        if not wd_fits(key, wanted, False):
            raise ValueError('CRT initializer arrays do not fit the original layout')
        wd_place(key, wanted)
        cursor = wanted + len(section.data)


    changed = True
    while changed:
        changed = False
        for key in sorted(writable - wd_positions.keys(),
                          key=lambda key: (-max(wd_votes[key].values(), default=0), key)):
            options = sorted(wd_votes[key], key=lambda offset: -wd_votes[key][offset])
            for offset in options:
                if wd_fits(key, offset, True, key in wd_known):
                    wd_place(key, offset)
                    changed = True
                    break
        for key, offset in list(wd_positions.items()):
            u, s = key
            section = units[u].obj.sections[s]
            for off, index, kind in section.relocs:
                if kind != coff.DIR32:
                    continue
                target = units[u].obj.symbols[index]
                target_u = u
                if target.storage == 2 or target.section == 0:
                    owner = owners.get(weak.get(target.name, target.name))
                    if owner is None:
                        continue
                    target_u, target_index = owner
                    target = units[target_u].obj.symbols[target_index]
                other = (target_u, target.section - 1)
                if other not in writable or other in wd_positions:
                    continue
                raw = image.read(data_base + offset + off, 4)
                if len(raw) != 4:
                    continue
                addend = int.from_bytes(section.data[off:off + 4], 'little', signed=True)
                address = int.from_bytes(raw, 'little')
                candidate = address - target.value - addend - data_base
                wd_votes[other][candidate] += 1




    changed = True
    while changed:
        changed = False
        for u, s in sorted(writable - wd_positions.keys()):
            section = units[u].obj.sections[s]
            if section.name != '.data' or not any(section.data) and not section.relocs:
                continue
            value = bytearray(section.data)
            for off, index, kind in section.relocs:
                address = operand_address(u, index) if kind == coff.DIR32 else None
                if address is None:
                    break
                addend = int.from_bytes(section.data[off:off + 4], 'little', signed=True)
                value[off:off + 4] = struct.pack('<I', (address + addend) & 0xffffffff)
            else:
                matches: list[int] = []
                cursor = 0
                while (found := data_original.find(value, cursor)) >= 0:
                    if wd_fits((u, s), found, True):
                        matches.append(found)
                    cursor = found + 1


                if len(matches) == 1 or matches and (u, s) not in referenced_storage:
                    wd_place((u, s), matches[0])
                    changed = True




    for (u, s), offset in wd_positions.items():
        section = units[u].obj.sections[s]
        if not same_code_pattern(section, reference.read(data_base + offset, len(section.data))):
            continue
        for off, index, kind in section.relocs:
            if kind != coff.DIR32:
                continue
            target = units[u].obj.symbols[index]
            target_u = u
            if target.storage == coff.CLASS_EXTERNAL or target.section == 0:
                owner = owners.get(weak.get(target.name, target.name))
                if owner is None:
                    continue
                target_u, index = owner
                target = units[target_u].obj.symbols[index]
            key = (target_u, target.section - 1)
            if key not in positions or key in placed:
                continue
            body = units[key[0]].obj.sections[key[1]]
            if not body.flags & coff.CODE:
                continue
            addend = int.from_bytes(section.data[off:off + 4], 'little', signed=True)
            address = int.from_bytes(reference.read(data_base + offset + off, 4), 'little')
            start = address - target.value - addend - text_base
            end = start + len(body.data)
            if (not 0 <= start < end <= len(original)
                    or not same_code_pattern(body, original[start:end])
                    or not calls_match(*key, start)):
                continue
            if any(start < at + len(units[k[0]].obj.sections[k[1]].data) and at < end
                   and not same_code_entry(body, start, units[k[0]].obj.sections[k[1]], at)
                   for k, at in placed.items()):
                continue
            placed[key] = start
            positions[key] = (1, start)
            text[start:end] = body.data

    exact_writable = len(wd_positions)

    for key in sorted(writable - wd_positions.keys(), key=lambda key: -len(units[key[0]].obj.sections[key[1]].data)):
        section = units[key[0]].obj.sections[key[1]]
        boundary = 1 << max(0, ((section.flags >> 20) & 15) - 1)
        size = len(section.data)
        low, high = (data_cut, len(wd_used)) if section.flags & 0x80 else (cursor, data_cut)
        offset = low
        while offset + size <= high:
            offset = align(data_base + offset, boundary) - data_base
            occupied = next((i for i in range(offset, offset + size) if wd_used[i]), -1)
            if occupied < 0:
                wd_place(key, offset)
                break
            offset = occupied + 1
        if key not in wd_positions:
            if section.flags & 0x80:
                offset = align(data_base + len(wd_used), boundary) - data_base
                wd_bss.extend(bytes(offset + size - data_cut - len(wd_bss)))
                wd_used.extend(bytes(offset + size - len(wd_used)))
                wd_place(key, offset)
            else:
                start = align(len(extra), boundary)
                extra.extend(bytes(start - len(extra)))
                extra.extend(section.data)
                positions[key] = (2, start)
    print('writable data:', exact_writable, 'anchored objects;', len(writable), 'total')



    simd = sorted(key for key in live if units[key[0]].obj.sections[key[1]].name == '.data1')
    simd_base, simd_size, _, _ = reference.sections['.data1']
    simd_original = image.read(simd_base, simd_size)
    simd_votes: dict[tuple[int, int], dict[int, int]] = defaultdict(lambda: defaultdict(int))
    for (u, s), start in placed.items():
        section = units[u].obj.sections[s]
        for off, index, kind in section.relocs:
            if kind != coff.DIR32:
                continue
            target = units[u].obj.symbols[index]
            key = (u, target.section - 1)
            if key in simd:
                address = int.from_bytes(original[start + off:start + off + 4], 'little')
                addend = int.from_bytes(section.data[off:off + 4], 'little', signed=True)
                simd_votes[key][address - target.value - addend - simd_base] += 1
    nonempty = [key for key in simd if units[key[0]].obj.sections[key[1]].data]
    if len(nonempty) > 8:
        raise ValueError('unexpected SIMD constant block count')
    best_score = (-1, -1)
    simd_data = bytearray(simd_size)
    simd_positions: dict[tuple[int, int], int] = {}
    for ordering in permutations(nonempty):
        candidate = bytearray()
        offsets: dict[tuple[int, int], int] = {}
        for key in ordering:
            section = units[key[0]].obj.sections[key[1]]
            boundary = 1 << max(0, ((section.flags >> 20) & 15) - 1)
            at = align(simd_base + len(candidate), boundary) - simd_base
            candidate.extend(bytes(at - len(candidate)))
            candidate.extend(section.data)
            offsets[key] = at
        if len(candidate) > simd_size:
            continue
        candidate.extend(bytes(simd_size - len(candidate)))
        score = (sum(a == b for a, b in zip(candidate, simd_original)),
                 sum(simd_votes[key].get(at, 0) for key, at in offsets.items()))
        if score > best_score:
            best_score = score
            simd_data = candidate
            simd_positions = offsets
    if nonempty and not simd_positions:
        raise ValueError('SIMD constants do not fit the original section')
    for key in simd:
        positions[key] = (6, simd_positions.get(key, 0))
    print('SIMD data:', best_score[0], '/', simd_size, 'bytes placed')



    code = {key for key in positions if units[key[0]].obj.sections[key[1]].flags & coff.CODE}
    tables = {key for key in readonly if units[key[0]].obj.sections[key[1]].flags & 0x1000}
    foldable = {key for key in code if positions[key][0] == 2}
    foldable.update(key for key in tables if not votes[key])
    aliases = fold_identical_sections(units, owners, weak, positions, code | tables, foldable)
    compact = bytearray()
    for key in sorted((key for key in positions if positions[key][0] == 2 and key not in aliases),
                      key=lambda key: (positions[key][1], key)):
        section = units[key[0]].obj.sections[key[1]]
        boundary = 1 << max(0, ((section.flags >> 20) & 15) - 1)
        offset = align(len(compact), boundary)
        compact.extend(bytes(offset - len(compact)))
        compact.extend(section.data)
        positions[key] = (2, offset)
    for key, previous in aliases.items():
        positions[key] = positions[previous]
    extra = compact
    print('folded helper copies:', len(aliases))

    sections: list[tuple[str, int, bytes, list[tuple[int, str, int]]]] = [
        ('.text', coff.TEXT, text, []), ('.extra', 0xe0300040, extra, []),


        ('.rdata', (coff.RDATA & ~0xf00000) | 0x400000, rd_data, []),
        ('.data', 0xc0300040, wd_init, []), ('.bss', 0xc0300080, wd_bss, []),
        ('.data1', 0xc0600040, simd_data, [])]
    for u, s in sorted(live):
        section = units[u].obj.sections[s]
        if section.flags & coff.CODE or (u, s) in readonly or (u, s) in writable or (u, s) in simd:
            continue
        positions[u, s] = (len(sections) + 1, 0)
        sections.append((section.name, section.flags & ~0x1000, section.data, []))


    definitions: dict[str, tuple[int, int]] = {}
    absolutes: dict[str, int] = {}
    for u, unit in enumerate(units):
        for i, sym in enumerate(unit.obj.symbols):
            if sym is None:
                continue
            key = (u, sym.section - 1)
            if sym.section > 0 and key in positions:
                if sym.storage == 2 and owners.get(sym.name) != (u, i):
                    continue
                sec, offset = positions[key]
                definitions[name(u, i)] = (sec, offset + sym.value)
            elif sym.section == -1:
                absolutes[name(u, i)] = sym.value

    common = bytearray()
    for symbol, (u, i) in owners.items():
        sym = units[u].obj.symbols[i]
        if sym.section == 0 and sym.value:
            start = align(len(common), min(16, 1 << (sym.value.bit_length() - 1)))
            common.extend(bytes(start + sym.value - len(common)))
            definitions[name(u, i)] = (len(sections) + 1, start)
    if common:
        sections.append(('.bss', 0xc0500080, common, []))



    for offset, u, s in trampolines:
        body = f'__layout_body_{u}_{s}'
        definitions[body] = positions[u, s]
        sections[0][3].append((offset + 1, body, coff.REL32))
        for i, sym in enumerate(units[u].obj.symbols):
            if sym and sym.section == s + 1 and sym.name in known:
                definitions[name(u, i)] = (1, offset)



    for (u, s), (other_u, other_s) in aliases.items():
        first = units[u].obj.sections[s]
        second = units[other_u].obj.sections[other_s]
        for (off, index, kind), (other_off, other_index, other_kind) in zip(first.relocs, second.relocs):
            symbol = name(u, index)
            other_symbol = name(other_u, other_index)
            target = definitions.get(symbol, absolutes.get(symbol, symbol))
            other_target = definitions.get(other_symbol, absolutes.get(other_symbol, other_symbol))
            if (off, kind, target) != (other_off, other_kind, other_target):
                raise ValueError('folded helper refers to different storage')



    for (u, s), (sec, offset) in positions.items():
        data = units[u].obj.sections[s].data
        if sections[sec - 1][2][offset:offset + len(data)] != data:
            raise ValueError(f'overlapping contributions: {units[u].path}, section {s}')

    referenced: set[str] = set()
    for u, s in sorted(live):
        sec, offset = positions[u, s]
        for off, i, typ in units[u].obj.sections[s].relocs:
            symbol = name(u, i)
            referenced.add(symbol)
            sections[sec - 1][3].append((offset + off, symbol, typ))
    referenced.update(r[1] for r in sections[0][3])
    undefined = sorted(referenced - definitions.keys() - absolutes.keys(),
                       key=lambda symbol: (known.get(symbol, known.get('__imp_' + symbol, 1 << 32)), symbol))
    for section in sections:
        relocs = section[3]
        unique: dict[tuple[int, int], tuple[str, object]] = {}
        for offset, symbol, kind in relocs:
            target = definitions.get(symbol, absolutes.get(symbol, symbol))
            previous = unique.get((offset, kind))
            if previous is not None and previous[1] != target:
                raise ValueError(f'conflicting relocations in {section[0]} at {offset:#x}: {previous} vs {(symbol, target)}')
            unique[offset, kind] = (symbol, target)
        relocs[:] = [(offset, value[0], kind) for (offset, kind), value in sorted(unique.items())]


    ordering = [i for i in range(1, len(sections) + 1) if sections[i - 1][0] != '.extra']
    ordering += [i for i in range(1, len(sections) + 1) if sections[i - 1][0] == '.extra']
    remap = {old: new for new, old in enumerate(ordering, 1)}
    sections = [sections[i - 1] for i in ordering]
    definitions = {symbol: (remap[section], at) for symbol, (section, at) in definitions.items()}
    positions = {key: (remap[section], at) for key, (section, at) in positions.items()}

    global _import_fixups
    _import_fixups = []
    for i, section in enumerate(sections, 1):
        for offset, symbol, kind in section[3]:
            if kind == coff.DIR32 and symbol.startswith('__imp_'):
                anchor = f'__th09_output_{i}'
                definitions[anchor] = (i, 0)
                addend = int.from_bytes(section[2][offset:offset + 4], 'little', signed=True)
                _import_fixups.append((anchor, offset, symbol, addend))
    indices = {symbol: i for i, symbol in enumerate([*definitions, *undefined, *absolutes])}
    for (u, s), (sec, offset) in positions.items():
        section = units[u].obj.sections[s]
        if section.name == '.sxdata':
            data = b''.join(struct.pack('<I', indices[name(u, index)]) for (index,) in struct.iter_unpack('<I', section.data))
            old = sections[sec - 1]
            sections[sec - 1] = (old[0], old[1], data, old[3])
    report = []
    for (u, s), (output_section, offset) in sorted(positions.items()):
        section = units[u].obj.sections[s]
        report.append({'object': Path(units[u].path).name, 'section': section.name,
                       'index': s, 'size': len(section.data), 'output': output_section,
                       'offset': offset, 'output_name': sections[output_section - 1][0], 'code': bool(section.flags & coff.CODE),
                       'symbols': [sym.name for sym in units[u].obj.definitions() if sym.section == s + 1]})
    (output_dir / 'layout.json').write_text(json.dumps(report, indent=2) + '\n')
    output = str(output_dir / 'th09.obj')
    coff.write_object(output, sections, definitions, undefined, absolutes)
    raw = bytearray(Path(output).read_bytes())
    symbol_offset = struct.unpack_from('<I', raw, 8)[0]
    for u, s in positions:
        section = units[u].obj.sections[s]
        if section.name == '.sxdata':
            for (old_index,) in struct.iter_unpack('<I', section.data):
                index = indices[name(u, old_index)]
                struct.pack_into('<H', raw, symbol_offset + index * 18 + 14, 0x20)
    Path(output).write_bytes(raw)
    print(f'{len(placed)} functions placed; {len(trampolines)} oversized; {len(extra)} extra code bytes; {len(undefined)} imports')
    return [output]


def exe_finished(features: Sequence[str]) -> None:

    image = compare_exe.PE(project.EXE)
    reference = compare_exe.PE(project.ORIGINAL)
    symbols = compare_exe.read_map()
    restored, changed = restore_imports(image, reference, _import_fixups, symbols)
    Path(project.EXE).write_bytes(restored)
    image = compare_exe.PE(project.EXE)
    if image.imports() != reference.imports():
        raise ValueError('restored imports do not match the original API bindings')
    pattern = re.compile(r'^(\s*[0-9a-f]{4}:)([0-9a-f]{8})(\s+)(\S+)(\s+)([0-9a-f]{8})', re.M)

    def moved_import(match: re.Match[str]) -> str:
        prefix, relative, spacing, symbol, gap, address = match.groups()
        if symbol.startswith('__layout_import_stub_'):
            return f'{prefix}{relative}{spacing}{symbol.removeprefix("__layout_import_stub_")}{gap}{address}'
        if symbol not in changed:
            return match.group(0)
        target = changed[symbol]
        position = int(relative, 16) + target - int(address, 16)
        return f'{prefix}{position:08x}{spacing}{symbol}{gap}{target:08x}'

    map_path = Path(project.EXE_MAP)
    map_path.write_bytes(pattern.sub(moved_import, map_path.read_bytes().decode('latin-1')).encode('latin-1'))
    if image.directory(4)[1] or image.directory(6)[1]:
        raise ValueError('section packing requires an unsigned image without debug data')
    alignment = struct.unpack_from('<I', image.data, image.opt + 36)[0]
    packed = bytearray(image.data[:image.sizeof_headers])
    order = {name: index for index, name in enumerate(reference.order)}
    for name in sorted(image.order, key=lambda name: (order.get(name, len(order)), name)):
        _, _, old_offset, size = image.sections[name]
        if not size:
            continue
        offset = align(len(packed), alignment)
        packed.extend(bytes(offset - len(packed)))
        packed.extend(image.data[old_offset:old_offset + size])
        header = image.opt + image.optsz + image.order.index(name) * 40
        struct.pack_into('<I', packed, header + 20, offset)
    Path(project.EXE).write_bytes(packed)
    result = compare_exe.PE(project.EXE)
    for name, (address, virtual_size, old_offset, size) in image.sections.items():
        new_address, new_size, offset, raw_size = result.sections[name]
        if (new_address, new_size, raw_size) != (address, virtual_size, size):
            raise ValueError(f'section packing changed virtual layout: {name}')
        if result.data[offset:offset + size] != image.data[old_offset:old_offset + size]:
            raise ValueError(f'section packing changed contribution bytes: {name}')
    if _table_object is None:
        raise ValueError('missing compiled table object for audit')
    count, pointers = check_tables(Path(project.EXE), Path(project.EXE_MAP),
                                   Path(project.ORIGINAL), _table_object)
    print(f'table audit: {count} complete tables, {pointers} pointer identities verified')
