# PRD-069: 实时数据对比 — DataDiffWidget hex/text侧边差异高亮

## 背景
调试串口通信协议时，工程师需要对比两次请求/响应的差异(如AT命令的响应变化、寄存器值的变更)。当前只能手动截图或复制文本比对，效率极低。实现DataDiffWidget，支持固定(Pin)当前数据快照，下次数据到来后自动对比显示，用颜色标记新增(绿)/删除(红)/未变(灰)内容。支持Hex和Text两种显示模式，从终端右键菜单或工具栏按钮触发。

## 需求列表
| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | DataDiffWidget: 双列布局 + 同步滚动 | P0 | terminal/widget/ |
| R2 | Pin机制: Pin当前响应 → Pin下一条 → 自动diff | P0 | terminal/widget/ |
| R3 | 差异高亮: Green=新增, Red=删除, Gray=未变 | P0 | terminal/widget/ |
| R4 | Hex/Text双模式显示 | P1 | terminal/widget/ |
| R5 | 终端右键菜单入口 + 工具栏按钮 | P0 | terminal/menu/ |
| R6 | Myers diff算法实现(轻量级) | P0 | utils/ |

## 接口设计

### DataDiffWidget类
```cpp
/**
 * @brief 数据对比组件 -- 双列同步滚动显示差异
 *
 * 工作流:
 *   1. 用户在终端右键选择"固定当前数据"(Pin)
 *   2. 新数据到来后，用户再次右键"与固定数据对比"
 *   3. DataDiffWidget弹出，左侧显示固定数据，右侧显示新数据
 *   4. 差异部分颜色标记: Green=新增, Red=删除, Gray=未变
 *   5. 支持Hex/Text模式切换
 */
class DataDiffWidget : public QWidget {
    Q_OBJECT

public:
    explicit DataDiffWidget(QWidget* parent = nullptr);

    /** @brief 设置对比数据
     * @param leftData 固定的原始数据
     * @param rightData 新数据
     * @param mode 显示模式(hex/text)
     */
    void setDiffData(const QByteArray& leftData, const QByteArray& rightData,
                     DisplayMode mode = DisplayMode::Text);

    /** @brief 切换显示模式 */
    void setDisplayMode(DisplayMode mode);

    enum class DisplayMode { Text, Hex };

signals:
    void closed();

private slots:
    void onThemeChanged();

private:
    void setupUi();
    void computeDiff();
    void updateDisplay();

    QTextEdit* m_leftView;
    QTextEdit* m_rightView;
    QScrollBar* m_syncScrollBar;    ///< 同步滚动条
    DisplayMode m_mode = DisplayMode::Text;

    QByteArray m_leftData;
    QByteArray m_rightData;

    /** @brief 差异行数据 */
    struct DiffLine {
        enum Type { Added, Removed, Unchanged };
        Type type;
        QString leftText;
        QString rightText;
    };
    QList<DiffLine> m_diffLines;
};
```

### Diff算法工具
```cpp
/**
 * @brief 轻量级diff算法 -- 基于Myers diff
 *
 * 逐行对比两个文本，返回DiffLine序列。
 * 不引入外部依赖，内嵌简化版Myers算法。
 */
namespace DiffAlgorithm {
    /** @brief 计算两段文本的差异
     * @param left 原始文本(按行拆分)
     * @param right 新文本(按行拆分)
     * @return 差异行序列
     */
    QList<DiffLine> compute(const QStringList& left, const QStringList& right);
}
```

### 集成入口
```cpp
// TerminalContextMenuManager注册菜单项
menu->addAction(tr("固定当前数据"), this, [this](){
    m_diffWidget->pinCurrentData(selectedText());
});
menu->addAction(tr("与固定数据对比"), this, [this](){
    m_diffWidget->setDiffData(m_pinnedData, selectedText());
    m_diffWidget->show();
});
```

## 依赖的公共组件
- ThemeManager (core/theme/ThemeManager.h) — 差异颜色(Success/Error/TextMuted)
- TerminalContextMenuManager (terminal/menu/TerminalContextMenuManager.h) — 右键菜单注册
- TerminalSelectionManager (terminal/selection/TerminalSelectionManager.h) — 获取选中数据
- ToolbarController (core/toolbar/ToolbarController.h) — 工具栏按钮

## 设计模式
- **策略模式**: DisplayMode切换Hex/Text显示策略
- **观察者模式**: 同步滚动通过信号连接两个QTextEdit的垂直滚动条
- **快照模式**: Pin操作创建数据快照，与后续数据对比

## 影响范围
| 文件 | 变更类型 | 风险 |
|------|---------|------|
| src/terminal/widget/DataDiffWidget.h | 新增 | 无 |
| src/terminal/widget/DataDiffWidget.cpp | 新增 | 无 |
| src/utils/diff/DiffAlgorithm.h | 新增 | 无 |
| src/utils/diff/DiffAlgorithm.cpp | 新增 | 无 |
| src/terminal/menu/TerminalContextMenuManager.cpp | 修改(菜单项) | 低 |
| src/core/toolbar/ToolbarController.cpp | 修改(按钮) | 低 |
| resources/themes/*.qss | 修改(DataDiffWidget样式) | 低 |

## 验收标准
1. 终端选中一段文本，右键"固定当前数据"，文本被保存为快照
2. 选中另一段文本，右键"与固定数据对比"，弹出双列对比窗口
3. 新增内容背景绿色(Success色)，删除内容背景红色(Error色)，未变内容灰色(TextMuted色)
4. 左右两列滚动同步，拖动任一侧滚动条另一侧跟随
5. Hex模式下显示为 "48 65 6C 6C 6F" 格式，差异按字节标记颜色
6. Text模式下差异按行标记颜色
7. 关闭对比窗口不丢失固定数据，可多次对比
8. .h ≤ 200行, .cpp ≤ 500行
9. 编译零错误
