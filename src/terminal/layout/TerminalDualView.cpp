/**
 * @file TerminalDualView.cpp
 * @brief 文本/十六进制双视图控件实现
 */

#include "terminal/layout/TerminalDualView.h"

#include <QSplitter>
#include <QVBoxLayout>

/**
 * @brief 构造函数
 *
 * 创建上方文本视图和下方十六进制视图，
 * 使用 QSplitter 垂直分隔。
 *
 * @param parent 父控件指针
 */
TerminalDualView::TerminalDualView(QWidget *parent)
    : QWidget(parent)
    , m_textView(nullptr)
    , m_hexView(nullptr)
    , m_splitter(nullptr)
{
    setObjectName(QStringLiteral("TerminalDualView"));
    setupUI();
}

/**
 * @brief 析构函数
 */
TerminalDualView::~TerminalDualView() = default;

/**
 * @brief 初始化界面布局
 *
 * 创建垂直分割的 QSplitter，上方放置文本视图终端控件，
 * 下方放置十六进制视图终端控件。
 * 使用占位 QWidget 代替实际的 TerminalWidget（待集成）。
 */
void TerminalDualView::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_splitter = new QSplitter(Qt::Vertical, this);

    // TODO: 替换为实际的 TerminalWidget 实例
    m_textView = nullptr; // 将由 TerminalWidget 创建
    m_hexView = nullptr;  // 将由 TerminalWidget 创建

    auto *textPlaceholder = new QWidget(m_splitter);
    textPlaceholder->setWindowTitle(tr("文本视图"));
    m_splitter->addWidget(textPlaceholder);

    auto *hexPlaceholder = new QWidget(m_splitter);
    hexPlaceholder->setWindowTitle(tr("十六进制视图"));
    m_splitter->addWidget(hexPlaceholder);

    layout->addWidget(m_splitter);
    setLayout(layout);
}
