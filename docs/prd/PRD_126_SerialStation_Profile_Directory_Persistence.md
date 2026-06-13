# PRD-126 Serial Station 默认档案目录持久化

## 背景

PRD-125 已支持通过 `--profile-dir` 或 `--serial-profile-dir` 固定 Serial Station 保存/加载档案对话框的默认目录。但该目录只存在于当前窗口生命周期内，用户下次从普通入口进入工作台时仍会回到系统文档目录。

企业工位通常有固定的线体配置目录。目录既可能来自启动脚本，也可能来自用户最近一次保存或加载档案的位置。为了减少重复定位文件的操作，Serial Station 需要记住上一次成功使用的默认档案目录，并在下一次打开窗口时自动恢复。

## 目标

1. `SerialProfileCatalogService` 持久化默认档案目录，继续复用 `SettingsManager`。
2. `SerialStationWindow` 构造时自动恢复已持久化的默认档案目录。
3. 启动参数设置的档案目录需要写入持久化配置。
4. 成功保存或加载 `.edserialprofile` 后，将该文件所在目录更新为默认档案目录。
5. 空白目录输入用于清除持久化目录，并让窗口恢复系统默认目录。
6. 默认目录变更不得清空最近档案、上次档案或删除真实档案文件。

## 非目标

1. 不扫描目录内所有档案。
2. 不自动创建目录。
3. 不改变 `.edserialprofile` 文件格式。
4. 不改变最近档案索引容量和排序规则。
5. 不提升真实串口设备验证等级，本轮仍为 D1 自动化测试。

## 用户故事

作为产线调试工程师，我希望 Serial Station 记住我上次使用的工位档案目录，这样每天打开工具后保存、加载配置时都能直接进入正确目录，而不需要反复浏览到同一个文件夹。

## 验收标准

1. `SerialProfileCatalogService::defaultProfileDirectory()` 初始为空。
2. `setDefaultProfileDirectory()` 能标准化并持久化非空目录。
3. 空白目录不会覆盖已有默认目录。
4. `clearDefaultProfileDirectory()` 只清除默认目录，不影响最近档案索引。
5. `clear()` 清空整个档案索引时同时清除默认目录。
6. 新建 `SerialStationWindow` 能读取已持久化的默认目录。
7. `saveCurrentProfileToFile()` 成功后默认目录更新为保存文件所在目录。
8. `loadProfileFromFile()` 成功后默认目录更新为加载文件所在目录。
9. `setDefaultProfileDirectory(QString())` 清除持久化目录并恢复系统默认目录。
10. README 中文企业级说明同步该工作流能力。

## 三轴目标

```text
Engineering: E5，服务层持久化、窗口恢复和保存/加载更新均有 QTest
User path:   U4，用户下次打开工具时自动回到上次工位档案目录
Device:      D1，不触碰真实串口连接验证
```
