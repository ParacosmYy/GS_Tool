# ADR-045：部署预检路径解析无副作用

## 状态

已接受（2026-08-10）。

## 问题

部署预检的职责是拒绝不安全配置，而不是初始化运行目录。原有 CLI 复用会创建父目录的
数据库路径 helper，虽然没有写数据库内容，却仍然让所谓只读预检改变了文件系统状态。

## 决策

- `db.get_db_path` 增加显式 `ensure_parent` 策略，默认保留初始化、连接和服务启动的兼容行为。
- `python -m token_tracker preflight` 传入 `ensure_parent=False`，只解析绝对路径并交给
  `deployment_checks` 检查；不会创建父目录、SQLite 文件、锁文件或备份。
- 真实初始化由 `init`/`create_app`/服务启动路径显式触发，目录创建仍集中在基础设施 helper，
  不在 CLI 控制器里复制。

## 取舍

- helper 多一个显式参数，但调用方能从代码直接看出是否允许文件系统准备动作，避免“只读”
  名称掩盖隐式 mkdir。
- 预检不再验证目录可写性或磁盘容量；这些属于正式部署演练，而不是无副作用配置检查。

## 验证

- 临时不存在路径运行 local preflight 后，目标数据库和父目录都保持不存在，命令成功退出。
- Python compile、CLI help、diff/行数门禁和既有启动 fail-closed 约束保持通过；没有读取真实数据库内容。
