# PRD-034: FrameParser 状态机拆分、DataExporter 导出增强与 UI 布局微调

## 背景

commit #33 完成了 MainWindow 持续瘦身、SettingsController 完善和 ToolbarController 增强。当前评分 35 分（PRD #33 已完成但尚未 commit），本迭代 #34。

**核心目标: 三线并进 -- 协议层代码质量提升、数据导出能力扩展、UI 一致性收尾。**

### 问题分析

**问题一: FrameParser.cpp 的 processByte() 方法 263 行，严重超出 80 行方法上限**

逐段审查 processByte() 方法（行 195~457）:

| 区段 | 行范围 | 行数 | 状态处理 | 可拆分方法 |
|------|--------|------|---------|-----------|
| 超时+长度上限检查 | 197~216 | 19 行 | 前置守卫 | 保留在 processByte() |
| Idle/HeaderMatching | 219~272 | 53 行 | 帧头匹配 | `handleHeaderMatching(byte)` |
| LengthReceiving | 275~296 | 21 行 | 长度字段接收 | `handleLengthReceiving(byte)` |
| PayloadReceiving | 298~388 | **90 行** | 数据接收+帧尾搜索 | `handlePayloadReceiving(byte)` |
| ChecksumVerifying | 390~413 | 23 行 | 校验验证 | `handleChecksumVerifying(byte)` |
| FooterMatching | 415~446 | 31 行 | 帧尾匹配 | `handleFooterMatching(byte)` |
| 尾部特殊情况 | 449~457 | 8 行 | 残留逻辑 | 合入 ChecksumVerifying |

processByte() 违反了 CLAUDE.md 4.6 节"单个方法不超过 80 行"的铁律。当前 263 行是上限的 3.3 倍。拆分后每个 handle 方法 20~50 行，processByte() 自身缩减至 20 行内的 switch 分发。

**问题二: DataExporter 当前仅支持终端数据导出（TXT/CSV/BIN），缺少协议解析数据和 HEX dump 导出**

当前 DataExporter（253 行 .cpp）只处理 `TerminalLine` 数据。实际需求:

| 导出场景 | 当前支持 | 说明 |
|---------|---------|------|
| 终端数据 → TXT（带时间戳+方向+HEX+ASCII） | 支持 | 已有 |
| 终端数据 → CSV（表头+逗号分隔） | 支持 | 已有 |
| 终端数据 → BIN（原始字节拼接） | 支持 | 已有 |
| 协议解析数据 → CSV（字段名+字段值） | **不支持** | 嵌入式调试常见需求 |
| 终端数据 → HEX dump 格式（地址+HEX+ASCII） | **不支持** | 类似 hexdump 命令输出 |
| 按时间范围过滤 | 仅全量模式支持 | 流式模式不支持 |

**问题三: UI 控件细节不一致**

| 问题 | 当前状态 | 影响 |
|------|---------|------|
| QuickCommandBar 按钮高度不统一 | 命令按钮 32px（minimumSize），连接按钮 36px | 视觉不对齐 |
| 编辑对话框按钮缺少 objectName | `onEditRequested()` 中的 addRowBtn/delRowBtn/buttons 无 objectName | QSS 无法选中 |
| 波形图控件无 QSS 样式 | ChartWidget 按钮和工具栏没有主题样式 | 暗色主题下按钮突兀 |

---

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | FrameParser 状态机拆分: processByte() 拆分为 handleIdleState/handleHeaderMatching/handleLengthReceiving/handlePayloadReceiving/handleChecksumVerifying/handleFooterMatching 六个私有方法 | P0 | protocol/FrameParser.h/cpp |
| R2 | DataExporter 协议数据 CSV 导出: 新增 exportParsedFrames() 方法，将 frameParsed 信号的 QVariantMap 列表导出为 CSV | P1 | utils/DataExporter.h/cpp |
| R3 | DataExporter HEX dump 格式导出: 新增 exportHexDump() 方法，输出类似 hexdump 命令的格式 | P1 | utils/DataExporter.h/cpp |
| R4 | DataExporter 按时间范围过滤增强: 流式导出支持时间范围过滤回调 | P2 | utils/DataExporter.h/cpp |
| R5 | QuickCommandBar 按钮高度统一: 所有按钮统一为 32px，添加按钮和编辑按钮一致 | P0 | serial/QuickCommandBar.cpp |
| R6 | 编辑对话框按钮添加 objectName: addRowBtn/delRowBtn/buttons 添加 objectName | P0 | serial/QuickCommandBar.cpp |
| R7 | ChartWidget 基础 QSS 样式: 三个主题文件添加 ChartWidget 控件样式 | P0 | resources/themes/*.qss |

---

## 需求详细说明

---

### R1: FrameParser 状态机拆分 (P0)

#### R1.1 问题分析

`processByte()` 方法当前 263 行，违反 CLAUDE.md 4.6 节"单个方法不超过 80 行"的铁律。方法内包含 6 个状态的处理逻辑，通过一个巨大的 switch-case 组织。虽然逻辑正确，但:

1. 任何单个状态的修改都需要在 263 行的方法中定位
2. 无法单独测试某个状态的处理逻辑
3. 代码审查时需要在大段代码中理解状态转换

#### R1.2 拆分方案

将 switch-case 的每个 case 分支提取为独立的私有方法:

```
processByte(byte)                     // ~20 行: 超时检查 + 长度上限检查 + switch 分发
  ├─ handleIdleState(byte)            // 保留在 processByte 内，仅一行状态初始化
  ├─ handleHeaderMatching(byte)       // ~50 行: 帧头字节序列匹配
  ├─ handleLengthReceiving(byte)      // ~20 行: 长度字段接收和解析
  ├─ handlePayloadReceiving(byte)     // ~60 行: 数据接收 + 帧尾搜索
  ├─ handleChecksumVerifying(byte)    // ~25 行: 校验字段验证
  └─ handleFooterMatching(byte)       // ~30 行: 帧尾字节匹配
```

#### R1.3 接口设计

```cpp
// protocol/FrameParser.h private 区域新增:

/**
 * @brief 处理 Idle 状态下的字节
 *
 * Idle 状态不做额外处理，直接进入 HeaderMatching。
 * 此方法不单独提取，因为 Idle 和 HeaderMatching 在当前实现中共用一个 case 分支。
 */

/**
 * @brief 处理帧头匹配状态下的字节
 * @param byte 当前接收的字节
 *
 * 逐字节匹配帧头字节序列。匹配成功后根据帧定义决定下一状态:
 *   - 有长度字段 → LengthReceiving
 *   - 有校验字段但无长度 → PayloadReceiving（按校验偏移计算长度）
 *   - 有帧尾但无长度 → PayloadReceiving（逐字节搜索帧尾）
 *   - 最简帧（仅帧头） → PayloadReceiving
 *
 * 匹配失败时重置缓冲区，并检查当前字节是否是新帧头的起始字节。
 */
void handleHeaderMatching(unsigned char byte);

/**
 * @brief 处理长度字段接收状态下的字节
 * @param byte 当前接收的字节
 *
 * 追加字节到缓冲区，当长度字段接收完成后:
 *   1. 调用 parseLengthField() 解析长度值
 *   2. 校验长度值是否合法（非负且不超过帧上限）
 *   3. 切换到 PayloadReceiving 状态
 *
 * 长度值非法时报告错误并重置状态机。
 */
void handleLengthReceiving(unsigned char byte);

/**
 * @brief 处理有效数据接收状态下的字节
 * @param byte 当前接收的字节
 *
 * 这是最复杂的状态处理，包含三种分支:
 *   1. 有长度字段: 按计算的总帧长度接收，收满后进入校验或完成
 *   2. 无长度字段有帧尾: 逐字节搜索帧尾匹配
 *   3. 都没有: 简单帧，由其他机制触发完成
 *
 * 所有分支都包含帧长度上限检查，超限时报告错误并重置。
 */
void handlePayloadReceiving(unsigned char byte);

/**
 * @brief 处理校验验证状态下的字节
 * @param byte 当前接收的字节
 *
 * 追加字节到缓冲区，当校验字段接收完成后:
 *   1. 调用 verifyChecksum() 验证校验值
 *   2. 校验通过 → 有帧尾则进入 FooterMatching，否则帧完成
 *   3. 校验失败 → 报告错误并重置
 */
void handleChecksumVerifying(unsigned char byte);

/**
 * @brief 处理帧尾匹配状态下的字节
 * @param byte 当前接收的字节
 *
 * 追加字节到缓冲区，当帧尾接收完成后:
 *   1. 比较缓冲区尾部字节与帧尾定义
 *   2. 匹配成功且校验通过 → 发射 frameParsed 信号
 *   3. 校验失败 → 发射 frameError("Checksum mismatch")
 *   4. 帧尾不匹配 → 发射 frameError("Footer mismatch")
 *   5. 无论结果如何都执行 reset()
 */
void handleFooterMatching(unsigned char byte);

/**
 * @brief 帧解析完成后的通用处理
 * @param rawFrame 完整帧原始数据
 *
 * 提取字段值、递增帧计数、发射 frameParsed 信号、重置状态机。
 * 被 handlePayloadReceiving/handleChecksumVerifying/handleFooterMatching 共用。
 */
void completeFrame(const QByteArray& rawFrame);

/**
 * @brief 帧解析错误后的通用处理
 * @param reason 错误原因
 * @param rawFrame 已接收的数据
 *
 * 递增错误计数、发射 frameError 信号、重置状态机。
 */
void failFrame(const QString& reason);
```

#### R1.4 processByte() 重构后的结构

```cpp
void FrameParser::processByte(unsigned char byte)
{
    // ---- 超时检查 ----
    checkTimeout();

    // ---- 帧长度上限检查 ----
    int effectiveMax = qMin(m_maxFrameLength, kMaxFrameSize);
    if (m_buffer.size() >= effectiveMax) {
        failFrame(QString("Frame exceeds max length (%1 bytes)").arg(effectiveMax));
        // 不 return: 当前字节可能是新帧起始
    }

    // ---- 状态分发 ----
    switch (m_state) {
    case State::Idle:
    case State::HeaderMatching:
        handleHeaderMatching(byte);
        break;
    case State::LengthReceiving:
        handleLengthReceiving(byte);
        break;
    case State::PayloadReceiving:
        handlePayloadReceiving(byte);
        break;
    case State::ChecksumVerifying:
        handleChecksumVerifying(byte);
        break;
    case State::FooterMatching:
        handleFooterMatching(byte);
        break;
    }
}
```

processByte() 从 263 行缩减至约 25 行。

#### R1.5 消除重复代码

当前代码中存在两处帧尾匹配逻辑（PayloadReceiving 中行 333~356 和 FooterMatching 中行 419~443），逻辑完全相同。拆分时提取为共用方法:

```cpp
/**
 * @brief 检查缓冲区尾部是否匹配帧尾
 * @return true=帧尾匹配，false=不匹配或数据不足
 */
bool matchFooter() const;
```

同理，帧完成处理（extractFields + frameCount++ + emit frameParsed + reset）在多处重复，提取为 `completeFrame()` 方法。帧错误处理（emit frameError + errorCount++ + reset）也提取为 `failFrame()` 方法。

#### R1.6 行数预估

| 文件 | 当前行数 | 变化后行数 | 说明 |
|------|---------|-----------|------|
| FrameParser.cpp | 457 | ~520 | 新增 6 个 handle 方法（~80 行新增注释和声明间距），但 processByte 从 263→25 行 |
| FrameParser.h | 218 | ~275 | 新增 8 个私有方法声明 + Doxygen 注释 |

**注意**: 总行数略有增加（~70 行），但代码结构大幅改善。每个方法都在 80 行限制内。processByte() 从 263 行降至 25 行。

---

### R2: DataExporter 协议数据 CSV 导出 (P1)

#### R2.1 需求背景

嵌入式调试中，用户配置了 FrameParser 后，解析出的数据以 `QVariantMap` 形式通过 `frameParsed` 信号传递。当前没有机制将这些解析结果批量导出为可分析的格式。用户需要将解析后的帧数据导出为 CSV，以便用 Excel/MATLAB/Python 进行后续分析。

#### R2.2 数据流

```
串口数据 → FrameParser → frameParsed(fields, rawFrame) → ProtocolView 表格展示
                                                       → [新增] ExportController 收集帧数据
                                                       → DataExporter::exportParsedFrames() 写 CSV
```

#### R2.3 接口设计

```cpp
// utils/DataExporter.h 新增:

/**
 * @brief 协议帧解析结果数据结构
 *
 * 由 frameParsed 信号的数据组装，用于批量导出。
 * 包含解析后的字段值和原始帧数据。
 */
struct ParsedFrameData {
    int frameIndex = 0;              ///< 帧序号（从 1 开始）
    QDateTime timestamp;             ///< 接收时间戳
    QVariantMap fields;              ///< 解析后的字段名→值映射
    QByteArray rawFrame;             ///< 原始帧字节数据
};

/**
 * @brief 导出协议帧解析结果为 CSV 文件
 *
 * CSV 格式:
 *   - 第一行: 列标题（frame_index, timestamp, field1, field2, ..., raw_hex）
 *   - 每行一帧: 帧序号, ISO 时间戳, 各字段值, 原始 HEX
 *   - 字段值按 QVariant::toString() 输出，数值保持原始精度
 *
 * @param filePath 输出文件路径
 * @param frames 帧解析结果列表
 * @param fieldNames 字段名列表（决定 CSV 列顺序）。
 *        如果为空，则从第一帧的 fields.keys() 自动提取
 * @return true=导出成功，false=失败（文件无法打开或数据为空）
 */
bool exportParsedFrames(const QString& filePath,
                        const QVector<ParsedFrameData>& frames,
                        const QStringList& fieldNames = QStringList());
```

#### R2.4 CSV 输出格式示例

```csv
frame_index,timestamp,温度,电压,raw_hex
1,2026-06-01T14:30:01.234,25.3,3300,AA550400194D0CE4
2,2026-06-01T14:30:01.536,25.5,3298,AA550400194F0CE6
3,2026-06-01T14:30:01.838,25.8,3302,AA55040019420CE8
```

#### R2.5 实现要点

1. 字段名列表优先使用调用者传入的 `fieldNames`；为空时从第一帧提取
2. 不同帧可能有不同字段集（字段定义变更），缺失字段填空字符串
3. `raw_hex` 列始终存在，使用 HexConverter::toHexString() 转换
4. 时间戳使用 ISO 8601 格式（`Qt::ISODateWithMs`），Excel 可直接解析
5. 数值字段不添加引号，字符串字段添加引号（防止 CSV 注入）

---

### R3: DataExporter HEX dump 格式导出 (P1)

#### R3.1 需求背景

嵌入式调试中，用户经常需要查看数据的 HEX dump 格式（类似 Linux `hexdump -C` 命令输出）。这种格式同时展示地址偏移、十六进制字节和 ASCII 字符，是分析二进制协议的标准格式。

#### R3.2 接口设计

```cpp
// utils/DataExporter.h 新增:

/**
 * @brief 导出终端数据为 HEX dump 格式
 *
 * 输出格式类似 hexdump -C:
 *   00000000  7f 45 4c 46 02 01 01 00  |.ELF....|
 *   00000010  00 00 00 00 00 00 00 00  |........|
 *
 * 每行 16 字节，左列为偏移地址，中间为 HEX，右侧为 ASCII。
 * 方向（RX/TX）通过行间注释标记:
 *   // [RX] 2026-06-01 14:30:01.234
 *   00000000  aa 55 03 00 19 4d 0c e4  |.U...M..|
 *   // [TX] 2026-06-01 14:30:01.536
 *   00000000  aa 55 03 00 19 4f 0c e6  |.U...M..|
 *
 * @param filePath 输出文件路径
 * @param lines 终端行数据（支持按时间范围过滤）
 * @param from 起始时间（无效表示不限制）
 * @param to 结束时间（无效表示不限制）
 * @return true=导出成功，false=失败
 */
bool exportHexDump(const QString& filePath,
                   const QVector<TerminalLine>& lines,
                   const QDateTime& from = QDateTime(),
                   const QDateTime& to = QDateTime());
```

#### R3.3 HEX dump 输出格式示例

```
EmbedDebug HEX Dump Export - 2026-06-01 14:30:01
================================================

// [RX] 2026-06-01 14:30:01.234
00000000  aa 55 03 00 19 4d 0c e4  |.U...M..|

// [TX] 2026-06-01 14:30:01.536
00000000  aa 55 03 00 19 4f 0c e6  |.U...O..|

// [RX] 2026-06-01 14:30:01.838
00000000  7f 45 4c 46 02 01 01 00  00 00 00 00 00 00 00 00  |.ELF............|
00000010  00 00 00 00 00 00 00 00  |........|
```

#### R3.4 实现要点

1. 每条 TerminalLine 的数据独立显示，方向和时间戳作为行间注释
2. 偏移地址基于当前行的数据偏移（非全局偏移），每条 TerminalLine 从 0 开始
3. ASCII 列中不可打印字符显示为 `.`
4. Format 枚举新增 `HexDump` 值，使 `exportToFile` 和 `exportStreamed` 也支持此格式

---

### R4: DataExporter 按时间范围过滤增强 (P2)

#### R4.1 需求背景

当前 `exportStreamed()` 不支持时间范围过滤（因为流式模式下无法在不加载全部数据的情况下高效过滤）。但在实际使用中，用户可能录制了数小时的数据，只需要导出其中几分钟的内容。

#### R4.2 方案设计

为 `exportStreamed()` 新增可选的时间过滤回调:

```cpp
// utils/DataExporter.h 修改:

/**
 * @brief 行数据过滤器回调
 *
 * 返回 true 表示保留该行，false 表示跳过。
 * 可用于时间范围过滤、方向过滤等条件。
 */
using LineFilter = std::function<bool(const TerminalLine&)>;

/**
 * @brief 创建时间范围过滤器
 * @param from 起始时间（无效表示不限制起始）
 * @param to 结束时间（无效表示不限制结束）
 * @return LineFilter 实例，可直接传给 exportStreamed
 */
static LineFilter timeRangeFilter(const QDateTime& from = QDateTime(),
                                  const QDateTime& to = QDateTime());

/**
 * @brief 批量流式导出（带过滤支持）
 *
 * @param filter 可选过滤器。为空时导出全部数据。
 *               过滤器在拉取批次数据后逐行判断，跳过的行不计入写入。
 *               注意: 过滤器不能减少拉取量（因为需要逐行判断），
 *               但可以减少写入量和输出文件大小
 */
bool exportStreamed(const QString& filePath, Format format,
                    LineProvider lineProvider,
                    int totalLines, int batchSize,
                    LineFilter filter = LineFilter());
```

#### R4.3 使用示例

```cpp
// 导出最近 5 分钟的数据
QDateTime fiveMinAgo = QDateTime::currentDateTime().addSecs(-300);
auto filter = DataExporter::timeRangeFilter(fiveMinAgo);

exporter.exportStreamed(path, DataExporter::Txt,
                        lineProvider, totalLines, 1000, filter);
```

#### R4.4 实现要点

1. `timeRangeFilter()` 返回一个捕获 from/to 的 lambda
2. 流式导出内部在写入循环中增加 filter 判断: `if (filter && !filter(line)) continue;`
3. 向后兼容: filter 参数默认为空 LineFilter()，不传时行为不变
4. Format 枚举需要新增 `HexDump` 值，所以 exportStreamed 的 switch 也要新增对应分支

---

### R5: QuickCommandBar 按钮高度统一 (P0)

#### R5.1 问题分析

当前 QuickCommandBar 中按钮高度不一致:

| 控件 | objectName | 当前高度 | 来源 |
|------|-----------|---------|------|
| 命令按钮 | quickCmdBtn_N | 32px | `setMinimumSize(80, 32)` 行 78 |
| 编辑按钮 | quickCmdEditBtn | 32px | `setFixedSize(56, 32)` 行 21 |
| 添加按钮 | quickCmdAddBtn | **32px** | `setFixedSize(32, 32)` 行 27 |

经实际代码审查，三个按钮高度已统一为 32px。但需求中提到"连接按钮 36px"，这是指 ToolbarController 中连接按钮的高度。QuickCommandBar 作为底部栏，其按钮高度应与工具栏按钮保持一致。

根据 CLAUDE.md 6.3 节布局规范:
- 工具栏高度: 36-40px
- 按钮最小高度: 28px
- 发送区域高度: 36-40px

QuickCommandBar 作为发送区域的一部分，按钮高度应与发送区域对齐，即 32px 保持不变。但需要确认添加按钮（32x32 正方形）与命令按钮（最小宽度 80px，高度 32px）在视觉上是否对齐。

#### R5.2 修改方案

确认所有 QuickCommandBar 按钮统一为 32px 高度。如果发现工具栏连接按钮为 36px，需要决定:

**方案 A（推荐）**: QuickCommandBar 保持 32px，与发送栏输入框高度一致。QuickCommandBar 不属于工具栏，不需要与工具栏对齐。

**方案 B**: QuickCommandBar 按钮提升至 36px，与工具栏连接按钮对齐。

采用方案 A，当前 32px 已经正确。本次仅确认和记录这一决策。

---

### R6: 编辑对话框按钮添加 objectName (P0)

#### R6.1 问题分析

`QuickCommandBar::onEditRequested()` 中创建的对话框按钮没有 objectName:

```cpp
// 行 133~135: 无 objectName
auto* addRowBtn = new QPushButton(tr("添加行"), &dlg);
auto* delRowBtn = new QPushButton(tr("删除行"), &dlg);
auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
```

QSS 选择器无法选中这些控件，导致对话框按钮在暗色主题下可能显示为默认样式。

#### R6.2 修改方案

为三个控件添加 objectName:

```cpp
auto* addRowBtn = new QPushButton(tr("添加行"), &dlg);
addRowBtn->setObjectName("editDlgAddRowBtn");

auto* delRowBtn = new QPushButton(tr("删除行"), &dlg);
delRowBtn->setObjectName("editDlgDelRowBtn");

auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
buttons->setObjectName("editDlgButtonBox");
```

同时在对话框自身也添加 objectName:

```cpp
QDialog dlg(window());
dlg.setWindowTitle(tr("编辑快捷指令"));
dlg.setObjectName("quickCommandEditDlg");
dlg.setMinimumSize(480, 320);
```

对应的 QSS 样式需要在三个主题文件中补充:

```css
/* 快捷指令编辑对话框 */
QDialog#quickCommandEditDlg {
    background-color: var(--bg-secondary);
}
QDialog#quickCommandEditDlg QPushButton {
    min-width: 60px;
    min-height: 28px;
    padding: 4px 12px;
}
```

---

### R7: ChartWidget 基础 QSS 样式 (P0)

#### R7.1 问题分析

ChartWidget 的按钮和控件已有 objectName:

| 控件 | objectName | 状态 |
|------|-----------|------|
| 暂停按钮 | chartPauseBtn | 无 QSS 样式 |
| 清除按钮 | chartClearBtn | 无 QSS 样式 |
| 窗口大小下拉框 | chartWindowCombo | 无 QSS 样式 |
| 状态标签 | chartStatusLabel | 无 QSS 样式 |

在暗色主题下，ChartWidget 的按钮可能显示为系统默认样式（浅色背景），与整体暗色主题不协调。

#### R7.2 修改方案

在三个主题文件中添加 ChartWidget 控件的基础样式:

```css
/* ===================== 波形图控件 ===================== */

/* 暂停/清除按钮 - 次要按钮风格 */
QPushButton#chartPauseBtn,
QPushButton#chartClearBtn {
    background-color: transparent;
    border: 1px solid var(--border);
    border-radius: 4px;
    color: var(--text-secondary);
    padding: 2px 8px;
    min-height: 24px;
}

QPushButton#chartPauseBtn:hover,
QPushButton#chartClearBtn:hover {
    background-color: var(--bg-hover);
    color: var(--text-primary);
    border-color: var(--accent);
}

QPushButton#chartPauseBtn:checked {
    background-color: var(--accent);
    color: #ffffff;
    border-color: var(--accent);
}

QPushButton#chartClearBtn:pressed {
    background-color: var(--accent-pressed);
}

/* 窗口大小下拉框 */
QComboBox#chartWindowCombo {
    /* 继承全局 ComboBox 样式，此处仅做微调 */
    min-width: 70px;
}

/* 状态标签 */
QLabel#chartStatusLabel {
    color: var(--text-muted);
    font-size: 11px;
}

/* 图表区域背景 */
QChartView {
    background-color: var(--bg-primary);
}
```

#### R7.3 Chart 自绘配色

ChartWidget 使用 Qt Charts 的 QChart，其背景和图例颜色不通过 QSS 控制。需要在 ChartWidget::setupUI() 中从 ThemeManager 获取语义色并设置:

```cpp
// chart/ChartWidget.cpp setupUI() 中新增:
m_chart->setBackgroundBrush(QBrush(QColor(ThemeManager::instance().color("BgPrimary"))));
m_chart->legend()->setLabelColor(QColor(ThemeManager::instance().color("TextSecondary")));

// 坐标轴颜色
m_xAxis->setLabelsColor(QColor(ThemeManager::instance().color("TextMuted")));
m_yAxis->setLabelsColor(QColor(ThemeManager::instance().color("TextMuted")));
m_xAxis->setTitleBrush(QBrush(QColor(ThemeManager::instance().color("TextSecondary"))));
m_yAxis->setTitleBrush(QBrush(QColor(ThemeManager::instance().color("TextSecondary"))));

// 网格线
m_xAxis->setGridLineColor(QColor(ThemeManager::instance().color("Border")));
m_yAxis->setGridLineColor(QColor(ThemeManager::instance().color("Border")));
```

---

## 接口设计

### 修改文件总览

| 文件 | 修改类型 | 修改内容 |
|------|---------|---------|
| `protocol/FrameParser.h` | 修改 | 新增 8 个私有方法声明（handleXxx/completeFrame/failFrame/matchFooter） |
| `protocol/FrameParser.cpp` | 修改 | processByte() 拆分为 switch 分发 + 6 个 handle 方法，消除重复代码 |
| `utils/DataExporter.h` | 修改 | 新增 ParsedFrameData 结构体、exportParsedFrames/exportHexDump 声明、Format 枚举新增 HexDump、新增 LineFilter 类型和 timeRangeFilter |
| `utils/DataExporter.cpp` | 修改 | 实现 exportParsedFrames/exportHexDump/exportHexDumpImpl 及流式版本的 HexDump 分支 |
| `serial/QuickCommandBar.cpp` | 修改 | 编辑对话框添加 objectName |
| `resources/themes/dark_terminal.qss` | 修改 | 新增 ChartWidget 控件样式和编辑对话框样式 |
| `resources/themes/modern_dark.qss` | 修改 | 同上 |
| `resources/themes/light.qss` | 修改 | 同上 |
| `chart/ChartWidget.cpp` | 修改 | setupUI() 中使用 ThemeManager 设置 Chart 配色 |

---

## 依赖的公共组件

| 组件 | 用途 |
|------|------|
| `HexConverter` | HEX dump 导出中的字节转十六进制字符串、协议数据导出中的 raw_hex 列 |
| `CRC` | FrameParser 校验逻辑（本次不修改，仅确认复用） |
| `ThemeManager` | ChartWidget 配色从语义色板获取 |
| `Constants` | kMaxFrameSize 等常量（本次不修改） |
| `FrameDefinition` | FrameParser 使用的帧定义（本次不修改） |
| `TerminalTypes` | TerminalLine 数据结构（DataExporter 已依赖） |

---

## 设计模式

| 模式 | 应用场景 |
|------|---------|
| **状态模式 (State)** | FrameParser 的 handleXxx 方法本质上是状态模式的实现，processByte() 是状态分发器 |
| **策略模式 (Strategy)** | LineFilter 回调是策略模式，允许调用者自定义过滤策略 |
| **模板方法 (Template Method)** | completeFrame/failFrame 是帧完成/失败的模板方法，统一了信号发射和状态重置流程 |
| **工厂方法 (Factory Method)** | timeRangeFilter() 是过滤器工厂，封装了创建时间范围过滤器的细节 |

---

## 影响范围

### 功能影响分析

| 功能 | 影响 | 风险 |
|------|------|------|
| 帧解析 | processByte 逻辑拆分为子方法，行为完全不变 | **低** -- 纯重构，无行为变化 |
| 终端数据导出 | 新增 HEX dump 格式，现有 TXT/CSV/BIN 不受影响 | 低 -- 新增功能 |
| 协议数据导出 | 全新功能，不影响现有代码 | 低 -- 新增功能 |
| QuickCommandBar 编辑对话框 | objectName 添加，可能影响 QSS 渲染 | 低 -- 纯属性设置 |
| ChartWidget 外观 | QSS 样式和 ThemeManager 配色可能改变视觉表现 | 中 -- 需验证三个主题下效果 |
| 流式导出 | 新增可选 filter 参数，默认行为不变 | 低 -- 向后兼容 |

### 编译影响

- CMakeLists.txt: 无新增文件，无需修改
- 无第三方依赖变化
- 无新增 Qt 模块依赖

---

## 验收标准总表

### P0 验收（必须全部通过才能 commit）

| 编号 | 验收项 | 通过条件 |
|------|--------|---------|
| AC-P0-01 | FrameParser.cpp 编译零错误 | cmake --build build 无错误 |
| AC-P0-02 | processByte() 行数 | <= 30 行（仅超时检查 + 长度检查 + switch 分发） |
| AC-P0-03 | 每个 handleXxx 方法行数 | <= 80 行 |
| AC-P0-04 | 帧解析功能回归 | 所有帧格式（仅帧头/帧头+帧尾/帧头+长度/帧头+校验/帧头+帧尾+校验/无帧头/JustFloat/FireWater）解析结果与重构前完全一致 |
| AC-P0-05 | 帧解析错误处理回归 | 超时/超长/校验失败/帧尾不匹配/长度非法均正确报告 |
| AC-P0-06 | QuickCommandBar 编辑对话框 objectName | quickCommandEditDlg/editDlgAddRowBtn/editDlgDelRowBtn/editDlgButtonBox 四个 objectName 设置正确 |
| AC-P0-07 | ChartWidget 暗色主题显示 | 暂停/清除按钮有正确的暗色样式，不是系统默认白色 |
| AC-P0-08 | ChartWidget 亮色主题显示 | 亮色主题下控件可读，无对比度问题 |
| AC-P0-09 | ChartWidget 图表区域背景 | 暗色主题下图表背景为深色，不是白色 |
| AC-P0-10 | DataExporter.cpp 编译零错误 | cmake --build build 无错误 |
| AC-P0-11 | EmbedDebug.bat 启动 | commit 后 bat 启动正常，无崩溃 |

### P1 验收（建议通过）

| 编号 | 验收项 | 通过条件 |
|------|--------|---------|
| AC-P1-01 | 协议帧 CSV 导出 | 导出的 CSV 可被 Excel 正确打开，列标题和数据对齐 |
| AC-P1-02 | 协议帧 CSV 空数据处理 | frames 为空时返回 false，不创建空文件 |
| AC-P1-03 | 协议帧 CSV 字段不一致处理 | 不同帧有不同字段时，缺失字段填空字符串 |
| AC-P1-04 | HEX dump 格式正确性 | 16 字节一行，偏移地址递增，ASCII 列正确 |
| AC-P1-05 | HEX dump 方向标记 | RX/TX 行间注释正确 |
| AC-P1-06 | HEX dump 流式导出 | exportStreamed 支持 HexDump 格式 |

### P2 验收（可选）

| 编号 | 验收项 | 通过条件 |
|------|--------|---------|
| AC-P2-01 | 流式导出时间过滤 | timeRangeFilter 创建的过滤器能正确过滤指定时间范围外的行 |
| AC-P2-02 | 流式导出无过滤兼容 | 不传 filter 时行为与修改前完全一致 |

---

## 实现优先级排序

按依赖关系排序:

```
第一批（无依赖，可并行）:
  R1 FrameParser 状态机拆分 ─────── 60 分钟 (纯重构，需仔细验证)
  R5 QuickCommandBar 按钮确认 ──── 10 分钟 (确认高度统一)
  R6 编辑对话框 objectName ─────── 15 分钟 (简单属性设置)

第二批（依赖第一批完成）:
  R2 协议帧 CSV 导出 ───────────── 40 分钟 (DataExporter 新增方法)
  R3 HEX dump 导出 ────────────── 45 分钟 (DataExporter 新增格式)
  R7 ChartWidget QSS 样式 ──────── 30 分钟 (三主题文件 + ThemeManager)

第三批（依赖第二批完成）:
  R4 流式导出时间过滤增强 ──────── 30 分钟 (修改 exportStreamed 接口)
```

**预估总工作量**: 约 3.5 小时

---

## 行数预估

### FrameParser.cpp 行数变化明细

| 操作 | 行数变化 | 说明 |
|------|---------|------|
| processByte() 缩减 | -238 行 | 从 263 行降至 25 行 |
| 新增 handleHeaderMatching() | +48 行 | 帧头匹配逻辑 |
| 新增 handleLengthReceiving() | +20 行 | 长度字段接收逻辑 |
| 新增 handlePayloadReceiving() | +55 行 | 数据接收+帧尾搜索逻辑 |
| 新增 handleChecksumVerifying() | +22 行 | 校验验证逻辑 |
| 新增 handleFooterMatching() | +25 行 | 帧尾匹配逻辑 |
| 新增 completeFrame() | +8 行 | 帧完成通用处理 |
| 新增 failFrame() | +8 行 | 帧错误通用处理 |
| 新增 matchFooter() | +12 行 | 帧尾匹配共用逻辑 |
| 方法分隔注释和空行 | +25 行 | Doxygen 注释和段落间距 |
| 删除尾部特殊情况代码 | -8 行 | 合入 ChecksumVerifying |
| **合计** | **+63 行** | **457 → ~520 行** |

### DataExporter 行数变化明细

| 操作 | 行数变化 | 说明 |
|------|---------|------|
| exportParsedFrames() 实现 | +55 行 | CSV 写入逻辑 |
| exportHexDump() 实现 | +65 行 | HEX dump 格式化逻辑 |
| exportStreamedHexDump() 实现 | +45 行 | 流式 HEX dump |
| timeRangeFilter() 实现 | +10 行 | 静态工厂方法 |
| exportStreamed 修改 | +10 行 | 新增 filter 参数和 HexDump 分支 |
| Format 枚举修改 | +1 行 | 新增 HexDump |
| **合计** | **+186 行** | **253 → ~440 行** |

### 总体行数变化

| 文件 | 当前行数 | 变化后行数 | 状态 |
|------|---------|-----------|------|
| FrameParser.cpp | 457 | ~520 | 在 500 行上限之上，但 processByte 已达标 |
| FrameParser.h | 218 | ~275 | 略超 200 行上限，新增方法声明较多 |
| DataExporter.cpp | 253 | ~440 | 接近 500 行上限 |
| DataExporter.h | 70 | ~110 | 正常 |
| QuickCommandBar.cpp | 266 | ~270 | 变化极小 |
| dark_terminal.qss | 不定 | +30 行 | 新增 ChartWidget 和编辑对话框样式 |
| modern_dark.qss | 不定 | +30 行 | 同上 |
| light.qss | 不定 | +30 行 | 同上 |
| ChartWidget.cpp | 263 | ~280 | 新增 ThemeManager 配色代码 |

**注**: FrameParser.cpp 从 457 行增至 ~520 行，看似变多，但核心改进是 processByte() 从 263 行降至 25 行。总行数增加来自方法声明注释和必要的函数间距，这是代码可维护性的合理代价。如果后续需要进一步控制行数，可以将 extractFields/verifyChecksum/parseLengthField/computeChecksum 等已有辅助方法提取到独立的 FrameParserUtils.cpp 中。

---

## 风险与缓解

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| FrameParser 拆分引入行为差异 | 帧解析结果错误 | 重构前后使用相同测试数据对比输出，逐状态验证 |
| handlePayloadReceiving 复杂度仍高 | 方法行数接近上限 | 该状态逻辑本身有三种分支（有长度/有帧尾/简单帧），可接受 |
| DataExporter 行数接近 500 上限 | 后续难以扩展 | 如果 R2/R3/R4 全部完成后超过 500 行，可将 HEX dump 导出提取到独立的 HexDumpExporter |
| ChartWidget ThemeManager 耦合 | 图表配色依赖主题系统 | 使用 ThemeManager::instance().color() 运行时查询，主题切换时需要刷新 |
| QSS ChartView 选择器可能不生效 | Qt Charts 内部渲染 | QChartView 是 QGraphicsView 子类，QSS 对其子控件控制有限，需要通过 C++ API 设置 |
| 编辑对话框 QSS 优先级 | 对话框是临时创建的，可能不继承主题 | 对话框的 parent 设置为 window()，应能继承主题样式 |

---

## 本迭代不涉及的内容

| 排除项 | 原因 |
|--------|------|
| FrameParser 的 extractFields/verifyChecksum/computeChecksum 提取 | 本次聚焦 processByte 拆分，这些方法已足够短 |
| DataExporter Excel (.xlsx) 原生导出 | 需要引入第三方库（如 QtXlsx），复杂度高，CSV 已满足 Excel 打开需求 |
| ProtocolView 集成协议帧导出 UI | 需要新增 UI 控件和信号连接，超出本迭代范围 |
| ChartWidget 主题切换时动态刷新配色 | 需要监听 ThemeManager::themeChanged 信号，作为后续迭代优化 |
