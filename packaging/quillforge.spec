# -*- mode: python ; coding: utf-8 -*-

from pathlib import Path

from PyInstaller.utils.hooks import collect_submodules


ROOT = Path(SPECPATH).parent
SOURCE = ROOT / "src"

analysis = Analysis(
    [str(SOURCE / "quillforge" / "__main__.py")],
    pathex=[str(SOURCE)],
    binaries=[],
    datas=[(str(ROOT / "assets" / "quillforge.ico"), "assets")],
    hiddenimports=collect_submodules("PyQt6.Qsci"),
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
)

pyz = PYZ(analysis.pure)

exe = EXE(
    pyz,
    analysis.scripts,
    analysis.binaries,
    analysis.datas,
    [],
    name="QuillForge",
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=False,
    console=False,
    icon=str(ROOT / "assets" / "quillforge.ico"),
    version=str(ROOT / "packaging" / "version_info.txt"),
)
