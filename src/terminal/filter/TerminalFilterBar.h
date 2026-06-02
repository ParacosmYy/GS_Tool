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

signals:
    /**
     * @brief 用户请求应用过滤
     * @param pattern 正则表达式
     * @param caseSensitive 是否区分大小写
     */
    void filterRequested(const QString &pattern, bool caseSensitive);

private slots:
    /**
     * @brief 处理应用按钮点击
     */
    void onApplyClicked();

private:
    QLineEdit *m_patternEdit;       ///< 正则表达式输入框
    QCheckBox *m_caseCheck;         ///< 大小写敏感复选框
    QPushButton *m_applyBtn;        ///< 应用过滤按钮
};

#endif // TERMINALFILTERBAR_H
