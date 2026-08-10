# ADR-038：固化 Windows Python 运行时依赖

## 状态

已接受（2026-08-10）。

## 背景

个人启动器、生产 WSGI 和 EXE 打包都依赖 Windows Python 环境。此前只有带宽范围的
`requirements.txt`，新机器或重建虚拟环境可能解析出不同的 Flask、Requests 或 Waitress
版本，削弱回滚和故障复现能力。

## 决策

- 保留 `requirements.txt` 作为直接依赖和升级意图说明；新增 `requirements.lock` 固化当前
  运行时及其已安装传递依赖的版本。
- `windows/start.bat` 和 EXE 打包脚本优先使用锁定文件；锁定文件缺失时启动器才回退到
  直接依赖文件，避免把旧 checkout 静默升级成无法复现的环境。
- PyInstaller 属于可选构建工具，当前不把它混入运行时锁；正式打包前必须在批准的构建
  环境单独审查并记录其版本。

## 取舍

- 锁文件固定版本但不等于供应链完整证明；它不包含哈希或签名，发布前仍需在具备
  `pip-audit`/供应链审查能力的环境执行原生依赖审计。
- 新版本安全升级需要显式更新锁文件、执行兼容性验证并形成独立提交，不能由双击入口
  自动完成。
- 当前变更只记录已有本地虚拟环境版本，不联网、不升级、不修改用户数据。

## 验证

- 锁文件与当前 `windows/.venv` 的 `pip freeze` 版本一致。
- `pip check`、Python 编译、CLI help 和文件规模门禁通过。
- `start.bat`、打包脚本和 README 使用同一个 runtime lock 输入。
