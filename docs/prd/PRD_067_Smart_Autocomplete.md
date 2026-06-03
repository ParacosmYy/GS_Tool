# PRD-067: 智能自动补全 — 发送栏基于历史记录的补全弹出

## 背景
当前SendController的发送栏输入命令时，用户需要手动输入或从下拉列表选择历史命令。下拉列表交互笨重，且无法根据当前输入实时过滤。实现一个轻量级补全弹出框(SendCompleter)，在输入时根据SendHistory实时匹配历史记录，前缀匹配并显示最多8条建议，↑↓选择，Tab/Enter确认，提升高频命令输入效率。

## 需求列表
| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | SendCompleter: 输入框下方弹出匹配列表 | P0 | serial/commands/ |
| R2 | 前缀匹配: 从SendHistory中筛选前缀一致项，最多8条 | P0 | serial/commands/ |
| R3 | ↑↓键导航，Tab/Enter接受补全 | P0 | serial/commands/ |
| R4 | Hex/Text模式图标区分(IconManager) | P1 | serial/commands/ |
| R5 | 与SendHistory/SeendController集成 | P0 | serial/commands/ |

## 接口设计

### SendCompleter类
```cpp
/**
 * @brief 发送补全弹出框 -- 基于SendHistory的前缀匹配补全
 *
 * 工作流:
 *   1. 监听SendInput的textChanged信号
 *   2. 查询SendHistory匹配当前前缀的条目
 *   3. QFrame弹出框定位在输入框正下方
 *   4. ↑↓选择，Tab/Enter接受，Esc关闭
 *   5. 选中后填入输入框并触发发送
 *
 * 定位: 跟随输入框位置，通过QPoint映射到全局坐标。
 * 焦点: 弹出时不抢夺输入框焦点(Qt::ToolTip标志)。
 */
class SendCompleter : public QFrame {
    Q_OBJECT

public:
    explicit SendCompleter(QLineEdit* input, SendHistory* history,
                           QWidget* parent = nullptr);

    /** @brief 完成弹出框定位(在输入框正下方) */
    void popup();

    /** @brief 关闭弹出框 */
    void hideCompleter();

signals:
    /** @brief 用户选中某个补全项
     * @param text 补全文本
     * @param isHex 是否HEX格式
     */
    void completionSelected(const QString& text, bool isHex);

private slots:
    void onInputTextChanged(const QString& text);

private:
    void setupUi();
    void keyPressEvent(QKeyEvent* event) override;
    void updateMatches(const QString& prefix);
    void selectItem(int index);
    void acceptCurrent();

    QLineEdit* m_input;            ///< 关联的输入框
    SendHistory* m_history;        ///< 发送历史数据源
    QListWidget* m_listWidget;     ///< 补全列表

    struct MatchItem {
        QString text;
        bool isHex;
    };
    QList<MatchItem> m_matches;    ///< 当前匹配结果
    int m_selectedIndex = -1;      ///< 当前选中索引(-1=无选中)
    const int MAX_MATCHES = 8;     ///< 最大显示条数
};
```

### SendController集成
```cpp
// SendController.h新增
SendCompleter* m_completer;  ///< 发送补全弹出框

// 构造函数中
m_completer = new SendCompleter(m_input, m_history, parent);
connect(m_completer, &SendCompleter::completionSelected,
        this, [this](const QString& text, bool isHex) {
    m_input->setText(text);
    send();  // 自动发送
});
```

## 依赖的公共组件
- SendHistory (serial/commands/SendHistory.h) — 历史记录数据源, recentTexts()
- SendController (serial/commands/SendController.h) — 发送逻辑, 集成入口
- ThemeManager (core/theme/ThemeManager.h) — 弹出框颜色
- IconManager (core/theme/IconManager.h) — Hex/Text模式图标

## 设计模式
- **观察者模式**: 监听QLineEdit::textChanged实时过滤
- **委托模式**: 匹配逻辑委托给SendHistory，SendCompleter只负责显示和交互
- **事件过滤**: 通过keyPressEvent拦截↑↓Tab Enter Esc键盘事件

## 影响范围
| 文件 | 变更类型 | 风险 |
|------|---------|------|
| src/serial/commands/SendCompleter.h | 新增 | 无 |
| src/serial/commands/SendCompleter.cpp | 新增 | 无 |
| src/serial/commands/SendController.h | 修改(+m_completer) | 低 |
| src/serial/commands/SendController.cpp | 修改(集成) | 低 |
| resources/themes/*.qss | 修改(SendCompleter样式) | 低 |

## 验收标准
1. 输入框输入"AT"时，弹出框显示以"AT"开头的历史命令(最多8条)
2. ↑↓键移动选中项(高亮BgHover)，Tab/Enter将选中项填入输入框
3. Esc关闭弹出框，不修改输入框内容
4. 弹出框定位在输入框正下方，不超出屏幕边界
5. 弹出框不抢夺输入框焦点(可继续输入过滤)
6. HEX格式项显示十六进制图标，Text格式显示文本图标
7. 空输入时不显示弹出框
8. .h ≤ 200行, .cpp ≤ 500行
9. 编译零错误
