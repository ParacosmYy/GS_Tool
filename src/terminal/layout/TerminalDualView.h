/**
 * @file TerminalDualView.h
 * @brief 文本/十六进制双视图控件
 *
 * 上方显示 ASCII 文本视图，下方显示十六进制视图，
 * 通过 QSplitter 分隔，可拖拽调整比例。
 */

#ifndef TERMINAL_DUAL_VIEW_H
#define TERMINAL_DUAL_VIEW_H

#include <QWidget>

class QSplitter;
class TerminalWidget;

/**
 * @class TerminalDualView
 * @brief 终端文本/十六进制双视图
 *
 * 将同一个数据源同时以文本形式和十六进制形式展示，
 * 两个视图通过垂直分割条分隔。
 */
class TerminalDualView : public QWidget
{
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit TerminalDualView(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~TerminalDualView() override;

private:
    /** @brief 初始化界面布局 */
    void setupUI();

    TerminalWidget *m_textView; ///< 文本视图终端控件
    TerminalWidget *m_hexView;  ///< 十六进制视图终端控件
    QSplitter *m_splitter;      ///< 垂直分割条
};

#endif // TERMINAL_DUAL_VIEW_H
