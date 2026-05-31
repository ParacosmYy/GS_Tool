# PRD-042: Bug Audit Sprint (Iteration #42)

## 背景
Iteration #42 的全面 Bug 审查。按照 CLAUDE.md 3.4.5 规定，每 3 次迭代执行一次 Bug 审计冲刺。
审查范围覆盖终端、串口、OTA、UI/UX 和 QSS 主题文件。逐文件逐行检查功能逻辑、线程安全、边界条件和视觉一致性。

## 需求列表

### P0 -- 功能完全不可用

| ID | 模块 | Bug 描述 | 文件:行 | 详情 |
|----|------|---------|---------|------|
| P0-01 | Terminal | 选中后 Ctrl+C 无法复制：keyPressEvent 中 Ctrl+C 判断缺少对 Ctrl 键的精确匹配，`event->modifiers() & Qt::ControlModifier` 使用位与而非完整匹配，当用户同时按下 Ctrl+Shift 时也会触发复制 | `src/terminal/TerminalWidget.cpp:365` | 应改为 `event->modifiers() == Qt::ControlModifier`。当前实现导致 Ctrl+Shift+C 组合键被拦截而无法传递到其他可能的快捷键处理器 |
| P0-02 | Terminal | 方向过滤模式下搜索高亮位置错误：searchMatches 中 match.line 是过滤后行号，但 paintLine 中传入的 displayLine 也是过滤后行号，搜索高亮计算使用了 `cached.text.left(match.startCol)` 做 xOffset 偏移，但 xOffset 已包含时间戳宽度，导致高亮矩形起始位置被双倍偏移 | `src/terminal/TerminalWidget.cpp:224` | 第 224 行 `xStart = xOffset + 4 + ...` 中 xOffset 已经是时间戳后的偏移量，与搜索 match 的 startCol 不对齐。当 `m_showTimestamp` 为 true 时，搜索高亮会向右偏移时间戳宽度 |
| P0-03 | AnimatedProgressBar | `stopShimmer()` 中 stop 后立即置 nullptr 但未 delete：动画对象使用 `DeleteWhenStopped` 启动，stop() 后 Qt 会异步 delete，但代码立即将 `m_shimmerAnim = nullptr`，导致若之后 `startShimmer()` 在 Qt 删除之前被调用，旧的动画对象仍在内存中但已无法追踪 | `src/ota/AnimatedProgressBar.h:98-100` | 应在 stop 前先 disconnect 或改用 finished 信号清理。不过由于 `startShimmer()` 内第一行调用 `stopShimmer()`，且 `DeleteWhenStopped` 在 stop 后会在事件循环中删除，时序上基本安全，但若 `stopShimmer` 后紧跟 `startShimmer` 且事件循环尚未处理删除，会短暂存在两个动画对象 |
| P0-04 | XModemTransfer | 等待启动信号阶段只响应 NAK/C，不响应 'C'(CRC_CHAR) 如果接收方同时发送了其他字符：当 `m_mode` 设置为 CRC 但接收方发送 NAK 时，代码在第 177 行强制切换为 Checksum 模式，覆盖了用户显式选择的 CRC 模式 | `src/ota/protocols/XModemTransfer.cpp:174-178` | 第 176-178 行：`if (m_mode != Checksum) { m_mode = Checksum; }` 这会忽略用户在 UI 中选择的 CRC 模式。若接收方先发 NAK 再发 C，发送方已切换为 Checksum 且不会恢复。应仅在没有显式设置模式时自动切换 |
| P0-05 | ConnectionController | 网络连接 `connectNetwork()` 中 `m_lastConnectParams` 未保存：导致自动重连时 `m_lastConnectType` 为非 Serial 但 `m_lastConnectParams` 为空，重连时 `connectSerial` 不会被调用，而 `connectNetwork` 使用硬编码的默认参数而非用户配置的参数 | `src/core/ConnectionController.cpp:231-232` | 第 232 行 `m_connectedPortName.clear()` 之后再无保存网络参数。自动重连 `onAutoReconnect()` 第 366 行调用 `connectNetwork(m_lastConnectType)` 但该方法每次都用硬编码的 127.0.0.1:8080 默认参数 |
| P0-06 | TerminalModel | `setMaxLines()` 在持有 mutex 时 emit `dataCleared()` 信号：虽然代码第 153 行调用了 `locker.unlock()` 后再 emit，但 `locker` 是 `QMutexLocker` 栈对象，第 153 行 `locker.unlock()` 后紧接着 `emit dataCleared()` 看起来正确，然而若连接到该信号的槽函数又调用 `setMaxLines()`，会因 mutex 未被锁定而允许重入，造成 `m_buffer` 的 resize 与 move 操作数据竞争 | `src/terminal/TerminalModel.cpp:153-156` | 虽然 mutex 已释放，但重入时 `m_buffer.resize(max)` 和 `std::move` 操作在两个调用栈中同时执行，由于 `m_buffer` 是共享状态，需要更高级别的防重入保护 |

### P1 -- 功能受限

| ID | 模块 | Bug 描述 | 文件:行 | 详情 |
|----|------|---------|---------|------|
| P1-01 | Terminal | 鼠标拖拽选择不支持向上滚动自动翻页：当鼠标拖拽到 TerminalWidget 顶部边缘以上时，不会自动向上滚动以扩展选区，用户只能通过鼠标滚轮先滚动再选择来选中大段文本 | `src/terminal/TerminalWidget.cpp:347-354` | `mouseMoveEvent` 中仅更新选区但无自动滚动逻辑。应在鼠标 Y < 0 或 Y > height() 时启动定时器自动滚动 |
| P1-02 | Terminal | 搜索高亮不支持列级选择：`searchMatches` 返回的 match 只有 line+startCol+length，但 paintLine 绘制时使用 `horizontalAdvance` 计算像素宽度，对于 Unicode/中文字符宽度计算可能不精确（Consolas 等宽字体中 CJK 字符占双宽） | `src/terminal/TerminalWidget.cpp:225` | `horizontalAdvance(cached.text.mid(match.startCol, match.length))` 在 CJK 字符存在时可能返回不精确的像素宽度，导致高亮框宽度错误 |
| P1-03 | SerialConfigPanel | DTR/RTS checkbox 在连接建立后仍可操作，但 toggle 信号直接转发到 `ConnectionController::setDtr/setRts`：若连接尚未就绪（连接中状态），DTR/RTS 设置会失败但无反馈给用户 | `src/serial/SerialConfigPanel.cpp:125-126,194-195` | DTR/RTS 在连接中状态时不应该允许操作，或者操作失败时应有 Toast 反馈 |
| P1-04 | TimedSender | `setData()` 和 `setDataQueue()` 无线程安全保护：`m_queue` 和 `m_queueIndex` 未加锁，若在定时器回调中（通过 `doSend()` 读取 `m_queue`）同时外部线程调用 `setData()` 修改 `m_queue`，会导致数据竞争 | `src/serial/TimedSender.cpp:63-79` | `doSend()` 在第 163 行读取 `m_queue[m_queueIndex]`，而 `setData()` 在第 65 行重置 `m_queue`。虽然 QTimer 回调在主线程，但 `onTimeout()` 通过 `invokeMethod(QueuedConnection)` 调度，时序上基本安全，但 `setData()` 可从任意线程调用，文档注释声称线程安全但实际并未对 `m_queue` 加锁 |
| P1-05 | TimedSender | `doSend()` 读取 `m_queue.size()` 未加锁：第 166 行 `m_queueIndex = (m_queueIndex + 1) % m_queue.size()` 与 `setData()` 的 `m_queue = {data}` 存在竞争 | `src/serial/TimedSender.cpp:166` | 若外部线程在 doSend 执行期间调用 setData，m_queue 可能已变，m_queueIndex 模运算会基于新 size 而非旧 size |
| P1-06 | OtaWidget | 双重验证：`onStartTransfer()` 第 208 行调用 `m_manager->validateFilePath()`，而 `m_manager->startTransfer()` 内部第 176 行也会调用 `validateFilePath()`，文件被打开验证了两次 | `src/ota/OtaWidget.cpp:208` + `src/ota/OtaManager.cpp:176` | 性能浪费，且 OtaWidget 中 `m_currentFileSize = fileInfo.size()` 在 HEX 文件场景下记录的是原始 HEX 大小而非转换后的 BIN 大小，导致历史记录中的 fileSize 不准确 |
| P1-07 | XModemTransfer | 1MB 文件大小限制过于保守：XMODEM-1K 模式使用 1024 字节块，理论上可传输更大文件，但代码硬编码 `kMaxFileSize = 1024 * 1024`（1MB），阻止了常见固件镜像（>1MB）的传输 | `src/ota/protocols/XModemTransfer.cpp:62` | 嵌入式固件经常超过 1MB（如 STM32H7 系列 flash 可达 2MB），应至少支持到 16MB 或与 `OtaManager::kMaxFirmwareSize` 对齐 |
| P1-08 | QuickCommandBar | `rebuildButtons()` 中使用 `deleteLater()` 删除旧按钮，但新按钮立即创建并添加到同一 layout：`deleteLater()` 是异步删除，旧按钮尚未销毁时新按钮已被添加，可能导致短暂的双重显示 | `src/serial/QuickCommandBar.cpp:97-99` | 应先 `delete` 旧按钮或确保 `deleteLater` 在事件循环处理完成后再添加新按钮。实际影响较小，因为 Qt 的 layout 管理器通常能处理，但在高 DPI 或复杂布局下可能出现视觉闪烁 |
| P1-09 | NavigationController | 面板切换动画的 `deleteLater` 清理时机问题：`switchToPanel()` 中 animGroup 使用 `connect(finished, deleteLater)` 清理，但 `m_switchAnimGroup` 在 finished 回调中被置为 nullptr。若动画中途 `NavigationController` 被析构，析构函数中 `delete m_switchAnimGroup` 会删除对象，而 `deleteLater` 在事件循环中再次 delete 同一指针导致 double-free | `src/core/NavigationController.cpp:339-340` + `~NavigationController:58-63` | 析构函数 `delete m_switchAnimGroup` 与 `deleteLater` 存在竞争。应在析构函数中断开 finished 信号连接后再 delete |
| P1-10 | ToastWidget | 静态 `activeToastsMap()` 中的 `QObject::destroyed` 连接使用 lambda 捕获 `parent` 指针，但 parent 被销毁时 activeToasts 列表中的 ToastWidget 子对象也会被 Qt 父子树自动销毁，导致 `deleteLater()` 对已销毁对象调用 | `src/core/ToastWidget.h:45` | `connect(parent, &QObject::destroyed, ...)` 中 `activeToasts(parent)` 仍然持有指向已销毁 toast 的指针。应改为在 destroyed 回调中先清理所有 toast 指针而不调用 deleteLater |
| P1-11 | ConnectionController | `connectSerial()` 中连接成功后未检查 `RecordingController` 是否为空就调用 `setConnected(true)`：虽然代码第 126-131 行有空指针检查，但若 `m_recordingController` 在连接期间被置为 nullptr（理论上不应该），第 268 行的调用会导致崩溃 | `src/core/ConnectionController.cpp:268` | `onConnectionStateChanged` 中 `if (m_recordingController)` 检查存在，但 `connectSignals` 连接的 lambda（第 418 行）中引用了 `m_connectedPortName` 和 `m_currentConn`，若在连接断开后仍收到 `errorOccurred` 信号，可能访问已清空的成员 |

### P2 -- 视觉/体验问题

| ID | 模块 | Bug 描述 | 文件:行 | 详情 |
|----|------|---------|---------|------|
| P2-01 | Terminal | 清屏操作只清空 widget 缓存但不清空 model 数据：`clear()` 方法只清空 `m_cachedLines` 和重置选区，但不会调用 `m_model` 的清空方法。这意味着清屏后如果新数据到来，旧数据仍然存在于 model 中 | `src/terminal/TerminalWidget.cpp:124-131` | `clear()` 发射 `clearRequested()` 信号由外部处理，但 TerminalWidget 自身的缓存清空与 model 清空存在时序差，用户可能在清屏后短暂看到旧数据闪烁 |
| P2-02 | Terminal | `formatToCache()` 对非 UTF-8 数据调用 `QString::fromUtf8()` 可能产生乱码：二进制数据（如协议帧）包含非 UTF-8 字节时，fromUtf8 会用 Unicode 替换字符替代，在 Mixed 和 Text 模式下显示为乱码方块 | `src/terminal/TerminalWidget.cpp:424,433` | 应在 Text 模式下对非法 UTF-8 字节做可读替换（如显示为 `\xNN`），或至少不静默吞掉数据 |
| P2-03 | AnimatedProgressBar | shimmer 效果在 `m_shimmerOffset` 接近 0.0 时被跳过：第 141 行 `if (m_shimmerOffset <= 0.0 ...)` 判断导致动画起始帧不绘制 shimmer，在循环动画中每隔 2000ms 会有一帧闪烁 | `src/ota/AnimatedProgressBar.h:141` | shimmer 偏移为 0 时应该绘制渐变（起始位置），当前逻辑导致动画循环点处出现可见的闪烁 |
| P2-04 | NavIndicatorWidget | 指示器绘制不考虑 header 的可变高度：第 163 行使用 `m_navTree->header()->height()` 计算 drawY 偏移，但未检查 header 是否隐藏（某些配置下 header 可被隐藏，此时 height() 返回 0 但 `visualRect` 已包含正确 Y 坐标） | `src/core/NavIndicatorWidget.h:163-164` | 当 header 隐藏时，`headerHeight` 为 0 不影响。但若 QTreeView 的 header 高度在主题切换后变化（如字体变化），指示器位置会在下一次 `moveToIndex` 之前保持旧值 |
| P2-05 | ToastWidget | toast 使用 `QFont("Segoe UI Emoji", ...)` 和 `QFont("Microsoft YaHei UI", ...)` 硬编码字体名：在非 Windows 系统或未安装这些字体的 Windows 版本上，图标和文字会回退到默认字体，视觉效果不一致 | `src/core/ToastWidget.h:103,107` | 应使用 ThemeManager 提供的字体配置或至少添加回退字体链 |
| P2-06 | SerialConfigPanel | 连接错误状态下 3 秒自动恢复使用 `QTimer::singleShot(3000)` 但不检查控件是否已被销毁：若 SerialConfigPanel 在 3 秒内被销毁（如窗口关闭），lambda 回调会访问已销毁的成员 | `src/serial/SerialConfigPanel.cpp:226-235` | 应使用 `QPointer` 保护或确保 SerialConfigPanel 的析构函数取消待处理的定时器。Qt 的 `singleShot` 在 receiver 被销毁时会自动取消（若使用了 context），当前代码使用 `this` 作为 context，Qt 6 会自动处理 |
| P2-07 | OtaWidget | 完成动画使用 `QPropertyAnimation("value", 100->100)` 作为定时器：第 407-409 行创建一个值不变的动画仅用于 400ms 定时，这是对 QPropertyAnimation 的误用，虽然功能正确但语义不清晰 | `src/ota/OtaWidget.cpp:407-409` | 应使用 `QTimer::singleShot(400)` 替代。当前实现会每帧触发进度条的 paintEvent（因为 value 属性变化通知），造成不必要的重绘 |
| P2-08 | QSS | 三个主题文件中 `QPushButton#connectBtn[state="error"]` 缺失样式定义：SerialConfigPanel 的 `setError()` 设置 `state="error"` 属性，但 QSS 只定义了 `state="connected"` 和 `state="connecting"`，错误状态会回退到默认按钮样式 | `resources/themes/dark_terminal.qss:454-476` | 三个主题文件均缺少 `QPushButton#connectBtn[state="error"]` 规则。错误状态下按钮应显示红色背景以匹配语义 |
| P2-09 | QSS | `QComboBox::down-arrow` 引用的 SVG 图标文件 `dropdown_arrow_light.svg` 和 `dropdown_arrow_light_disabled.svg` 在 light.qss 中使用，但 dark_terminal.qss 和 modern_dark.qss 使用 `dropdown_arrow.svg` 和 `dropdown_arrow_disabled.svg`。若资源文件中缺少这些 SVG，下拉箭头不显示 | `resources/themes/light.qss:282-288` | 需确认所有引用的 SVG 图标文件都存在于 `resources/icons/` 目录中 |
| P2-10 | QSS | `QLabel#driverInfoLbl` 在三个主题文件中被定义了两次：一次在 SerialConfigPanel 控件区域，一次在 PortWatcher 区域。第二次定义覆盖第一次，导致第一个定义中的 `font-size: 11px` 被覆盖 | `resources/themes/dark_terminal.qss:478-482,970-977` | 两个规则集略有不同（第一个有 `padding: 4px`，第二个有 `padding: 4px 8px` 和 `background-color: transparent`）。应合并为单一规则 |

## 影响范围

| 模块 | P0 | P1 | P2 | 总计 |
|------|-----|-----|-----|------|
| Terminal | 2 | 2 | 2 | 6 |
| Serial | 0 | 2 | 0 | 2 |
| OTA | 1 | 2 | 2 | 5 |
| UI/Core | 1 | 4 | 3 | 8 |
| QSS Theme | 0 | 0 | 3 | 3 |
| TerminalModel | 1 | 0 | 0 | 1 |
| **合计** | **5** | **10** | **10** | **25** |

## 验收标准

1. P0 Bug 全部修复后方可继续新特性开发（CLAUDE.md 铁律）
2. P1 Bug 在本次迭代或下次迭代中修复
3. P2 Bug 记录在案，按优先级择机修复
4. 每个修复需附带回归测试用例
5. 修复后所有三个主题 QSS 文件保持同步

## 修复优先级建议

**第一批（本次迭代必须修复）**:
- P0-01: Ctrl+C 精确匹配 -- 一行改动
- P0-05: 网络连接参数保存 -- 中等改动
- P1-07: XModem 1MB 限制 -- 改常量即可
- P2-08: connectBtn error 状态 QSS -- 补充三行规则

**第二批（下次迭代修复）**:
- P0-04: XModem 模式自动切换逻辑
- P1-04/P1-05: TimedSender 线程安全
- P1-06: OTA 双重验证
- P0-06: TerminalModel 重入保护

**第三批（后续迭代修复）**:
- P0-02: 搜索高亮偏移修复（需要仔细计算像素）
- P1-01: 拖拽自动滚动
- P1-09: 动画析构安全性
- P1-10: Toast destroyed 生命周期
