"""跨平台打包规格。"""

from __future__ import annotations

from dataclasses import dataclass, field

from embeddebug.serial_station.platform_packaging.config import PlatformConfig

_GUI_PLATFORMS = {"windows", "macos"}


@dataclass
class PackageSpec:
    """PyInstaller 打包规格。"""
    platform: PlatformConfig
    app_name: str
    version: str
    entry_point: str
    hidden_imports: list[str] = field(default_factory=list)
    data_files: list[str] = field(default_factory=list)
    excludes: list[str] = field(default_factory=list)
    onedir: bool = True

    def to_pyinstaller_args(self) -> list[str]:
        args: list[str] = ["--noconfirm"]
        args.append("--onedir" if self.onedir else "--onefile")
        args.extend(["--name", self.app_name])
        for hidden in self.hidden_imports:
            args.extend(["--hidden-import", hidden])
        for data in self.data_files:
            args.extend(["--add-data", data])
        for exclude in self.excludes:
            args.extend(["--exclude-module", exclude])
        if self.platform.os_name in _GUI_PLATFORMS:
            args.append("--windowed")
        args.append(self.entry_point)
        return args

    def validate(self) -> list[str]:
        errors: list[str] = []
        if not self.app_name.strip():
            errors.append("app_name 不能为空")
        if not self.version.strip():
            errors.append("version 不能为空")
        if not self.entry_point.strip():
            errors.append("entry_point 不能为空")
        if self.version.strip() and any(ch in self.version for ch in ("\\", "/", ":")):
            errors.append("version 不能包含路径分隔符")
        return errors
