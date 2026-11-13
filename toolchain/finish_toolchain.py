
import importlib.util
from pathlib import Path

root=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('vsdownload',root/'vsdownload.py')
vs=importlib.util.module_from_spec(spec)
spec.loader.exec_module(vs)
names=['Windows SDK Desktop Headers x86-x86_en-us.msi',
       'Windows SDK Desktop Libs x86-x86_en-us.msi',
       'Windows SDK for Windows Store Apps Headers-x86_en-us.msi',
       'Windows SDK for Windows Store Apps Libs-x86_en-us.msi']
cache=root/'cache/Win10SDK_10.0.19041-10.0.19041.4'
dest=root/'msvc'
vs.unpackWin10SDK(str(cache),[dict(fileName=n) for n in names],str(dest/'unpack'))
vs.moveVCSDK(str(dest/'unpack'),str(dest))
print('Microsoft x86 compiler and SDK extracted.')
