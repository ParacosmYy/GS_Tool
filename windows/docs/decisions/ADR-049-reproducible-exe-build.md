# ADR-049：Windows EXE 构建工具链锁定

## 状态

已接受（2026-08-10）。

## 背景

项目需要保留“个人双击体验”的 EXE 交付形态，但运行时依赖锁不能自动锁住可选的冻结工具。
每次构建临时安装未指定版本的 PyInstaller，可能改变 bootloader、hook 和资源收集行为，导致
同一源码无法得到可审计的构建结果。

## 决策

- 使用 `windows/packaging/requirements-build.lock` 独立锁定 PyInstaller 及其 Windows 构建依赖；
  运行时仍由 `windows/requirements.lock` 管理，两个边界不混成生产运行依赖。
- `packaging/build.ps1` 只消费两个锁文件，不再安装未锁定的 `pyinstaller`；`-Clean` 只允许清理
  `windows/build` 和 `windows/dist`，并在递归删除前检查目标仍位于 `windows/` 内。
- 继续使用 PyInstaller `--onedir` 和 `--add-data` 收集模板/静态资源；冻结入口通过 `sys.frozen`
  分支把数据库放到 `%LOCALAPPDATA%\AITokenTracker`，避免向临时解包目录写入用户数据。
- 真实 EXE 构建、启动、数据持久化、升级和签名仍属于批准构建环境的发布门禁；源码中的脚本和锁文件
  不能代替构建产物证据。

## 官方依据

- [PyInstaller 使用手册](https://pyinstaller.org/en/stable/usage.html)：`--onedir` 与 `--add-data` 的
  资源收集边界。
- [PyInstaller 运行时信息](https://pyinstaller.org/en/stable/runtime-information.html)：冻结进程的
  `sys.frozen` 运行时识别和资源路径行为。

## 验证边界

- 已完成：锁文件、构建脚本、清理路径保护、README 和 计划同步；本机 PyInstaller 未安装。
- 待完成：在批准的 Windows 构建环境执行锁定安装、EXE 生成、启动、数据库写入、升级和签名检查。
