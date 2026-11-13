
from pathlib import Path


def members(path: Path) -> list[tuple[str, bytes]]:
    raw = path.read_bytes()
    if raw[:8] != b'!<arch>\n':
        raise ValueError(f'{path}: not a COFF archive')
    result: list[tuple[str, bytes]] = []
    names = b''
    pos = 8
    while pos + 60 <= len(raw):
        header = raw[pos:pos + 60]
        if header[58:60] != b'`\n':
            raise ValueError(f'{path}: invalid member header at {pos:#x}')
        size = int(header[48:58])
        if size < 0 or pos + 60 + size > len(raw):
            raise ValueError(f'{path}: truncated member at {pos:#x}')
        name = header[:16].decode().strip()
        body = raw[pos + 60:pos + 60 + size]
        if name == '//':
            names = body
        elif name != '/':
            if name.startswith('/'):
                offset = int(name[1:])
                ends = [end for marker in (b'\0', b'/\n') if (end := names.find(marker, offset)) >= 0]
                if not ends:
                    raise ValueError(f'{path}: invalid long member name')
                end = min(ends)
                name = names[offset:end].decode()
            result.append((name.rstrip('/').replace('\\', '/').split('/')[-1], body))
        pos += 60 + size + size % 2
    return result
