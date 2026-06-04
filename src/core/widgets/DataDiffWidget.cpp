/**
 * @file DataDiffWidget.cpp
 * @brief 数据对比视图实现 — 双列LCS差异高亮
 *
 * 实现细节:
 *   - 使用LCS(最长公共子序列)算法计算行级差异
 *   - 通过HTML渲染差异高亮，避免QPainter自绘复杂度
 *   - 颜色全部通过ThemeManager语义色获取
 *   - QSplitter左右分栏，可拖拽调整比例
 */

#include "core/widgets/DataDiffWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QSplitter>
#include <QScrollBar>
#include <algorithm>

#include "core/theme/ThemeManager.h"

// ============================================================================
// 构造
// ============================================================================

/** @brief 构造数据对比视图，初始化左右分栏和同步滚动 @param parent 父控件指针 */
DataDiffWidget::DataDiffWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("dataDiffWidget");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    // 统计标签
    m_statsLabel = new QLabel(this);
    m_statsLabel->setObjectName("diffStatsLabel");
    m_statsLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statsLabel);

    // 水平分割器
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName("diffSplitter");

    // 左列
    auto* leftWidget = new QWidget(this);
    leftWidget->setObjectName("diffLeftPanel");
    auto* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(2);

    m_leftLabel = new QLabel(tr("原始数据"), leftWidget);
    m_leftLabel->setObjectName("diffLeftLabel");
    leftLayout->addWidget(m_leftLabel);

    m_leftEdit = new QTextEdit(leftWidget);
    m_leftEdit->setObjectName("diffLeftEdit");
    m_leftEdit->setReadOnly(true);
    leftLayout->addWidget(m_leftEdit);

    // 右列
    auto* rightWidget = new QWidget(this);
    rightWidget->setObjectName("diffRightPanel");
    auto* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(2);

    m_rightLabel = new QLabel(tr("当前数据"), rightWidget);
    m_rightLabel->setObjectName("diffRightLabel");
    rightLayout->addWidget(m_rightLabel);

    m_rightEdit = new QTextEdit(rightWidget);
    m_rightEdit->setObjectName("diffRightEdit");
    m_rightEdit->setReadOnly(true);
    rightLayout->addWidget(m_rightEdit);

    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setSizes({1, 1});
    mainLayout->addWidget(splitter);

    // 同步滚动
    connect(m_leftEdit->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int val) {
        m_rightEdit->verticalScrollBar()->setValue(val);
    });
}

// ============================================================================
// 公开接口
// ============================================================================

/** @brief 设置左右两侧数据并执行LCS差异比较 @param leftData 原始数据(左侧) @param rightData 当前数据(右侧) */
void DataDiffWidget::setData(const QString& leftData, const QString& rightData)
{
    const QStringList leftLines = leftData.split('\n');
    const QStringList rightLines = rightData.split('\n');

    ++m_totalComparisons;
    m_totalBytesCompared += static_cast<quint64>(leftData.size() + rightData.size());

    m_diffResult = computeDiff(leftLines, rightLines);
    refreshDisplay();
}

/** @brief 设置左列标题标签文字 @param label 左列标题 */
void DataDiffWidget::setLeftLabel(const QString& label)
{
    if (m_leftLabel) {
        m_leftLabel->setText(label);
    }
}

/** @brief 设置右列标题标签文字 @param label 右列标题 */
void DataDiffWidget::setRightLabel(const QString& label)
{
    if (m_rightLabel) {
        m_rightLabel->setText(label);
    }
}

/** @brief 清空所有差异结果和显示内容 */
void DataDiffWidget::clear()
{
    m_diffResult.clear();
    m_added = 0;
    m_removed = 0;
    m_modified = 0;

    m_leftEdit->clear();
    m_rightEdit->clear();
    m_statsLabel->clear();
}

// LCS diff算法/显示刷新/颜色映射/统计重置见 DataDiffWidgetDiff.cpp
