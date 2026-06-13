# PRD-121 - Serial Station Profile Catalog

## 背景

PRD-119 已支持保存/加载 `.edserialprofile`，PRD-120 已支持启动参数直接加载档案。真实工站仍缺少企业级配置体验：用户每次都要重新浏览文件路径，无法在工作台内看到最近档案，也无法一键恢复上次工位配置。

## 目标

1. Serial Station 记录最近成功保存、加载、启动应用的配置档案路径。
2. 工作台工具栏展示最近档案下拉入口。
3. 提供“重载上次”入口，快速恢复上一份成功应用的档案。
4. 最近档案使用项目既有配置管理能力持久化，不新增第二套配置文件格式。
5. 档案内容读写继续复用 `SerialProfileService`，本轮只新增路径索引服务。

## 非目标

1. 不实现档案编辑器、档案重命名、删除文件或目录扫描。
2. 不自动连接串口。
3. 不改变 `.edserialprofile` JSON schema。
4. 不提升真实硬件验证状态。

## 架构边界

- `SerialProfileCatalogService` 放在 `src/apps/serial_station/services/`，只管理路径索引。
- `SerialProfileCatalogService` 复用 `SettingsManager` 读写应用配置。
- `SerialStationWindow` 只装配工具栏、刷新下拉项和转发加载动作。
- `SerialProfileService` 仍是档案 JSON 的唯一读写服务。
- UI 不直接操作 `SettingsManager`。

## 验收标准

1. service QTest 覆盖最近档案去重、MRU 排序、容量裁剪、空路径忽略、清理。
2. Workbench QTest 覆盖保存/加载/启动加载成功后会记录最近档案。
3. Workbench QTest 覆盖新窗口能看到最近档案下拉和“重载上次”按钮。
4. Workbench QTest 覆盖“重载上次”能恢复 UART、协议、命令状态。
5. Workbench QTest 覆盖档案加载失败不会污染最近档案。
6. 主目标构建通过。
7. `EmbedDebug.bat --station serial` 启动探针通过。

## 三轴状态目标

- 工程状态：`E5`，service + workbench 路径有自动化测试，边界清晰。
- 用户状态：Serial Station 档案配置体验维持并增强到 `U4`，最近/上次档案成为日常入口。
- 设备状态：不提升，仍为 `D1` 自动化测试；真实硬件未验证。
