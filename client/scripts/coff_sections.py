
from bisect import bisect_right

import coff


def split_storage(obj: coff.Object) -> dict[int, list[tuple[int, int]]]:






    old_sections = list(obj.sections)
    starts: dict[int, list[int]] = {}
    for section_index, section in enumerate(old_sections):
        if (section.name not in ('.data', '.bss', '.rdata')
                and not section.name.startswith('.CRT')) or not section.data:
            continue
        values = {0}
        values.update(sym.value for sym in obj.symbols if sym and sym.section == section_index + 1
                      and 0 <= sym.value < len(section.data) and not sym.name.startswith('.'))
        starts[section_index] = sorted(values)



    for section in old_sections:
        data = bytearray(section.data)
        relocs: list[tuple[int, int, int]] = []
        for offset, index, kind in section.relocs:
            target = obj.symbols[index]
            cuts = starts.get(target.section - 1)
            if cuts and kind in (coff.DIR32, coff.REL32):
                addend = int.from_bytes(data[offset:offset + 4], 'little', signed=True)
                point = target.value + addend
                if 0 <= point <= len(old_sections[target.section - 1].data):
                    first = bisect_right(cuts, target.value) - 1
                    last = bisect_right(cuts, point) - 1
                    if first != last:
                        index = len(obj.symbols)
                        obj.symbols.append(coff.Symbol(f'__storage_{index}', point, target.section, coff.CLASS_STATIC))
                        data[offset:offset + 4] = bytes(4)
            relocs.append((offset, index, kind))
        section.data = bytes(data)
        section.relocs = relocs

    parts: dict[int, list[tuple[int, int]]] = {}
    obj.sections = []
    for section_index, section in enumerate(old_sections):
        cuts = starts.get(section_index, [0])
        parts[section_index] = []
        for start, end in zip(cuts, cuts[1:] + [len(section.data)]):
            parts[section_index].append((start, len(obj.sections) + 1))
            relocs = [(offset - start, index, kind) for offset, index, kind in section.relocs
                      if start <= offset < end]
            boundary = 1 << max(0, ((section.flags >> 20) & 15) - 1)
            if start:
                boundary = min(boundary, start & -start)
            flags = (section.flags & ~0xf00000) | (boundary.bit_length() << 20)
            obj.sections.append(coff.Section(section.name, section.data[start:end], relocs, flags))
    for symbol in obj.symbols:
        if symbol and symbol.section > 0:
            choices = parts[symbol.section - 1]
            start, section_index = choices[bisect_right([start for start, _ in choices], symbol.value) - 1]
            symbol.section = section_index
            symbol.value -= start
    return parts
