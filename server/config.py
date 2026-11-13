import os
from pathlib import Path


def read_env(path):
    path=Path(path)
    if not path.exists():
        return {}
    result={}
    for line in path.read_text(encoding='utf-8-sig').splitlines():
        line=line.strip()
        if not line or line.startswith('#'):
            continue
        if line.startswith('export '):
            line=line[7:].lstrip()
        key,separator,value=line.partition('=')
        if not separator or not key.strip():
            raise ValueError('Invalid .env configuration')
        value=value.strip()
        if value[:1] in ('"',"'"):
            if len(value)<2 or value[-1]!=value[0]:
                raise ValueError('Invalid .env configuration')
            value=value[1:-1]
        else:
            value=value.partition('#')[0].strip()
        result[key.strip()]=value
    return result


def settings(path=None):
    source=path or os.environ.get('TH09_ENV_FILE') or Path(__file__).resolve().parents[1]/'.env'
    return dict(read_env(source),**os.environ)


def port_number(value):
    text=str(value)
    if not text.isascii() or not text.isdigit() or not 1<=int(text)<=65535:
        raise ValueError('TH09_RELAY_PORT must be 1-65535')
    return int(text)


def client_endpoint(host=None,port=None,path=None):
    values=settings(path)
    host=(host if host is not None else values.get('TH09_RELAY_HOST','')).strip()
    if not host:
        raise ValueError('Set TH09_RELAY_HOST in .env')
    if any(character.isspace() or character in '/\\:@#' for character in host):
        raise ValueError('TH09_RELAY_HOST must be a hostname or IPv4')
    return host,port_number(port if port is not None else values.get('TH09_RELAY_PORT','10900'))


def bind_endpoint(host=None,port=None,path=None):
    values=settings(path)
    return host or values.get('TH09_BIND_HOST','0.0.0.0'),port_number(port if port is not None else values.get('TH09_RELAY_PORT','10900'))
