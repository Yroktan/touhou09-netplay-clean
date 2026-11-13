USAGE="Extracts the Platform SDK import libraries of Visual Studio .NET 2002.\n\n    psdk_libs.py ZIP [OUT]\n\nZIP is the Visual Studio .NET 2002 Professional media: a zip holding\ncd1/VSPROD1.iso. OUT defaults to the Wine prefix's\nC:\\msvc7\\PlatformSDK\\Lib.\n\nThe original games link import libraries such as user32, gdi32, winmm,\nkernel32 and uuid from Vc7\\PlatformSDK\\Lib (import library build 9210\nin the Rich header), not from a later Platform SDK. psdk.libs in\nproject.toml lists the ones a game needs, by their file names on the\nmedia (User32.Lib). These libraries are not loose files on the CDs:\nthey come with the merge module zWin32_b.common.msm, a cabinet embedded\nin vs_setup.msi on disc 1. msi_extract.py places them by the MSI's\ntables, which Wine's msidb exports.\n"
















import os, shutil, subprocess, sys, tempfile

import msi_extract, project

ROOT = project.ROOT
LIBS = project.config["psdk"]["libs"]
SDK = "Program Files/Microsoft Visual Studio .NET/Vc7/PlatformSDK/lib/"
CAB = "_1162_zWin32_b.common.msm"  


def run(*cmd, **kw):
    subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, **kw)


def main():
    if len(sys.argv) not in (2, 3):
        sys.exit(USAGE)
    if not LIBS:
        sys.exit("psdk.libs in project.toml lists no libraries")
    zip_path = os.path.abspath(sys.argv[1])
    prefix = os.environ.get("WINEPREFIX", os.path.join(ROOT, ".wine"))
    out = os.path.abspath(sys.argv[2] if len(sys.argv) > 2
                          else os.path.join(prefix, "drive_c/msvc7/PlatformSDK/Lib"))
    with tempfile.TemporaryDirectory(dir=os.path.join(ROOT, "build")) as tmp:
        run("7z", "e", "-y", "-o" + tmp, zip_path, "cd1/VSPROD1.iso")
        run("7z", "e", "-y", "-o" + tmp, os.path.join(tmp, "VSPROD1.iso"), "vs_setup.msi")
        os.remove(os.path.join(tmp, "VSPROD1.iso"))
        msi = os.path.join(tmp, "vs_setup.msi")
        cabs = os.path.join(tmp, "cabs")
        run("7z", "e", "-y", "-tCompound", "-o" + cabs, msi, CAB)
        os.rename(os.path.join(cabs, CAB), os.path.join(cabs, "zWin32.cab"))
        tables = os.path.join(tmp, "tables")
        os.makedirs(tables)
        env = dict(os.environ, WINEPREFIX=prefix, WINEDEBUG="-all")
        win = subprocess.run(["winepath", "-w", tables], env=env, check=True,
                             capture_output=True, text=True).stdout.strip()
        run("msidb", "-d", msi, "-f", win, "-e", "File", "Component", "Directory", env=env)
        staged = os.path.join(tmp, "out")
        sys.argv = ["msi_extract.py", tables, cabs, staged, *(SDK + l for l in LIBS)]
        msi_extract.main()
        os.makedirs(out, exist_ok=True)
        for lib in LIBS:
            shutil.copyfile(os.path.join(staged, SDK, lib), os.path.join(out, lib.lower()))
            print(os.path.join(out, lib.lower()))


if __name__ == "__main__":
    main()
