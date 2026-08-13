# Packaging

scripts/package.ps1 is the single Windows packaging entry point. It reads the
version from src/serialforge/__init__.py, generates the PyInstaller version
resource, builds into an isolated staging directory, inspects the embedded
archive, and writes a provenance manifest before declaring success.

~~~powershell
.\scripts\package.ps1 -Mode onedir
.\scripts\package.ps1 -Mode onefile
.\scripts\package.ps1 -Mode onedir -Ble
.\scripts\package.ps1 -Mode onefile -Ble
~~~

The default output is isolated by version, capability variant, and packaging
mode:

~~~text
dist\release\0.1.0\core\onedir\app\SerialForge\SerialForge.exe
dist\release\0.1.0\core\onefile\app\SerialForge.exe
dist\release\0.1.0\ble\onedir\app\SerialForge\SerialForge.exe
dist\release\0.1.0\ble\onefile\app\SerialForge.exe
~~~

Each mode directory also contains PROVENANCE.json, SHA256SUMS.txt,
PYINSTALLER_ARCHIVE.txt, DEPENDENCY_TREE.txt, NOTICE.txt, and
THIRD_PARTY_NOTICES.md. The manifest records the source revision, lockfile
hash, toolchain, artifact hash, PE version fields, Authenticode status,
capabilities, hardware-acceptance status, and forbidden-content scan. The
current bundle is an unsigned engineering build; the license file is an
inventory until the complete license-text bundle and legal review are finished.

The core package does not collect Bleak/WinRT. Use -Ble only for the explicit
BLE-enabled variant. RTT remains attach-only and last-stage: no SEGGER/J-Link
driver, SDK, DLL, executable, or other vendor binary is packaged.

The script prefers locked uv execution. A synchronized project .venv is
accepted for core packaging as a local fallback; CI always uses uv.
