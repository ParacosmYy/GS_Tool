/**
 * @file TerminalFilterBar.h
 * @brief 终端过滤工具栏
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 提供正则过滤输入框、大小写复选框和应用按钮的 UI 面板。
 */

#ifndef TERMINALFILTERBAR_H
#define TERMINALFILTERBAR_H

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

/**
 * @class TerminalFilterBar
 * @brief 终端过滤器输入栏，收集用户正则过滤参数
 */
class TerminalFilterBar : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父控件
     */
    explicit TerminalFilterBar(QWidget *parent = nullptr);

    /**
     * @brief 获取当前输入的正则表达式
     * @return 正则表达式字符串
     */
    QString currentPattern() const;

    /**
     * @brief 是否区分大小写
     * @return 大小写敏感状态
     */
    bool isCaseSensitive() const;

    /**
     * @brief 是否启用反转过滤
     * @return true 反转匹配（排除匹配行）
     */
    bool isInverted() const;

    // ---- 统计计数接口 ----
    /** @brief 获取累计过滤变更次数 */
    quint64 totalFilterChanges() const;
    /** @brief 获取累计高亮切换次数 */
    quint64 totalHighlightToggles() const;
    /** @brief 获取累计方向变更次数 */
    quint64 totalDirectionChanges() const;
    /** @brief 重置所有过滤统计计数器 */
    void resetFilterStatistics();

signals:
    /**
     * @brief 用户请求应用过滤
     * @param pattern 正则表达式
     * @param caseSensitive 是否区分大小写
     * @param inverted 是否反转过滤
     */
    void filterRequested(const QString &pattern, bool caseSensitive, bool inverted);

    /** @brief 用户请求清除过滤 */
    void filterCleared();

private slots:
    /**
     * @brief 处理应用按钮点击
     */
    void onApplyClicked();

private:
    QLineEdit *m_patternEdit;       ///< 正则表达式输入框
    QComboBox *m_historyCombo;      ///< 过滤历史下拉框
    QCheckBox *m_caseCheck;         ///< 大小写敏感复选框
    QCheckBox *m_invertCheck;       ///< 反转过滤复选框
    QPushButton *m_applyBtn;        ///< 应用过滤按钮
    QPushButton *m_clearBtn;        ///< 清除过滤按钮

    // ---- 统计计数器 ----
    quint64 m_totalFilterChanges = 0;       ///< 累计过滤变更次数
    quint64 m_totalHighlightToggles = 0;    ///< 累计高亮切换次数
    quint64 m_totalDirectionChanges = 0;    ///< 累计方向变更次数
};

#endif // TERMINALFILTERBAR_H
