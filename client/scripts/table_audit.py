





from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import hashlib
import re
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import coff
import compare_exe
import project

TARGET_SHA256 = '10350095bcf95edb59e03bee9849a2dc8a7714b4927ad5909c569c550fce6822'


@dataclass(frozen=True)
class Table:
    name: str
    address: int
    count: int
    stride: int

    @property
    def size(self) -> int:
        return self.count * self.stride


def contracts(path: Path = ROOT / 'src/exe/data_tables.h') -> list[Table]:
    result: list[Table] = []
    for line in path.read_text().splitlines():
        if not line.strip() or line.startswith('//'):
            continue
        match = re.fullmatch(r'TH09_TABLE\((\w+), (0x[0-9a-f]+), (\d+), (\d+)\)', line)
        if not match:
            raise ValueError(f'invalid table contract: {line}')
        name, address, count, stride = match.groups()
        result.append(Table(name, int(address, 16), int(count), int(stride)))
    if not result or len({row.name for row in result}) != len(result):
        raise ValueError('empty or duplicate table contracts')
    ordered = sorted(result, key=lambda row: row.address)
    for first, second in zip(ordered, ordered[1:]):
        if first.address + first.size > second.address:
            raise ValueError(f'overlapping table contracts: {first.name}, {second.name}')
    return result


def owns(symbol: str, name: str) -> bool:
    return symbol == '_' + name or symbol.startswith('?' + name + '@')


def linked_address(symbol: str, symbols: dict[str, tuple[int, str]]) -> int:
    if symbol in symbols:
        return symbols[symbol][0]
    candidates = {address for name, (address, _) in symbols.items()
                  if name.endswith('_' + symbol)}
    if len(candidates) != 1:
        raise ValueError(f'missing or ambiguous linked symbol: {symbol}')
    return candidates.pop()


def check_tables(executable: Path, map_path: Path, original: Path,
                 object_path: Path) -> tuple[int, int]:
    if hashlib.sha256(original.read_bytes()).hexdigest() != TARGET_SHA256:
        raise ValueError('table audit requires the original Japanese TH09 1.50a')
    reference, image = compare_exe.PE(str(original)), compare_exe.PE(str(executable))
    symbols = compare_exe.read_map(str(map_path))
    obj = coff.Object(str(object_path))
    tables = contracts()
    known = {symbol: address for address, _, _, names in compare_exe.read_symbols()
             for symbol in names}
    definitions: dict[str, coff.Symbol] = {}
    for table in tables:
        matches = [sym for sym in obj.symbols if sym and sym.section > 0 and owns(sym.name, table.name)]
        if len(matches) != 1:
            raise ValueError(f'{table.name}: expected one initialized definition')
        definitions[table.name] = matches[0]
        known[matches[0].name] = table.address
    pointers = 0
    for table in tables:
        definition = definitions[table.name]
        section = obj.sections[definition.section - 1]
        start, end = definition.value, definition.value + table.size
        following = [sym.value for sym in obj.symbols if sym
                     and sym.section == definition.section and sym.value > start
                     and not sym.name.startswith('.')]
        if end > min(following, default=len(section.data)):
            raise ValueError(f'{table.name}: truncated table, need {table.count} rows')
        address = linked_address(definition.name, symbols)
        if address != table.address:
            raise ValueError(f'{table.name}: wrong address {address:#x}, expected {table.address:#x}')
        expected = reference.read(table.address, table.size)
        actual = image.read(address, table.size)
        if len(expected) != table.size or len(actual) != table.size:
            raise ValueError(f'{table.name}: incomplete file-backed storage')
        mask = bytearray(table.size)
        for offset, index, kind in section.relocs:
            if not start <= offset < end:
                continue
            if kind != coff.DIR32 or offset + 4 > end:
                raise ValueError(f'{table.name}: unsupported table relocation')
            pos = offset - start
            mask[pos:pos + 4] = b'\1' * 4
            target = obj.symbols[index]
            addend = struct.unpack_from('<I', section.data, offset)[0]
            wanted = struct.unpack_from('<I', expected, pos)[0]
            observed = struct.unpack_from('<I', actual, pos)[0]
            if target.name in known:
                if wanted != (known[target.name] + addend) & 0xffffffff:
                    raise ValueError(f'{table.name}+{pos:#x}: wrong target identity {target.name}')
                resolved = (linked_address(target.name, symbols) + addend) & 0xffffffff
                if observed != resolved:
                    raise ValueError(f'{table.name}+{pos:#x}: linked pointer differs from {target.name}')
            elif target.name.startswith(('??_C@', '$SG')) and target.section > 0:
                storage = obj.sections[target.section - 1].data
                at = target.value + addend
                literal = storage[at:storage.index(b'\0', at) + 1]
                if reference.read(wanted, len(literal)) != literal or image.read(observed, len(literal)) != literal:
                    raise ValueError(f'{table.name}+{pos:#x}: string target differs')
            else:
                raise ValueError(f'{table.name}+{pos:#x}: unreviewed target {target.name}')
            pointers += 1
        for pos, (wanted, observed, compiled, marked) in enumerate(
                zip(expected, actual, section.data[start:end], mask)):
            if not marked and (wanted != observed or wanted != compiled):
                raise ValueError(f'{table.name}[{pos // table.stride}]+{pos % table.stride:#x}: data differs')
    return len(tables), pointers


def main() -> int:
    try:
        count, pointers = check_tables(Path(project.EXE), Path(project.EXE_MAP),
                                       Path(project.ORIGINAL), Path(project.config['exe']['obj_dir']) / 'exe_tables.obj')
    except (OSError, ValueError) as exc:
        print(f'table audit: {exc}', file=sys.stderr)
        return 1
    print(f'table audit: {count} complete tables, {pointers} pointer identities verified')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
