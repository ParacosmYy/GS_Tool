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

void DataDiffWidget::setData(const QString& leftData, const QString& rightData)
{
    const QStringList leftLines = leftData.split('\n');
    const QStringList rightLines = rightData.split('\n');

    ++m_totalComparisons;
    m_totalBytesCompared += static_cast<quint64>(leftData.size() + rightData.size());

    m_diffResult = computeDiff(leftLines, rightLines);
    refreshDisplay();
}

void DataDiffWidget::setLeftLabel(const QString& label)
{
    if (m_leftLabel) {
        m_leftLabel->setText(label);
    }
}

void DataDiffWidget::setRightLabel(const QString& label)
{
    if (m_rightLabel) {
        m_rightLabel->setText(label);
    }
}

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

// ============================================================================
// LCS diff算法
// ============================================================================

QVector<DiffLine> DataDiffWidget::computeDiff(const QStringList& left,
                                               const QStringList& right) const
{
    const int m = left.size();
    const int n = right.size();

    // 构建LCS动态规划表
    QVector<QVector<int>> dp(m + 1, QVector<int>(n + 1, 0));
    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            if (left[i - 1] == right[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = qMax(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    // 回溯生成差异序列
    int i = m, j = n;
    QVector<DiffLine> reversed;
    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && left[i - 1] == right[j - 1]) {
            reversed.append({DiffLineType::Unchanged, left[i - 1], right[j - 1], i});
            --i; --j;
        } else if (j > 0 && (i == 0 || dp[i][j - 1] >= dp[i - 1][j])) {
            reversed.append({DiffLineType::Added, QString(), right[j - 1], j});
            --j;
        } else {
            reversed.append({DiffLineType::Removed, left[i - 1], QString(), i});
            --i;
        }
    }

    std::reverse(reversed.begin(), reversed.end());
    return reversed;
}

// ============================================================================
// 显示刷新
// ============================================================================

void DataDiffWidget::refreshDisplay()
{
    m_added = 0;
    m_removed = 0;
    m_modified = 0;

    QString leftHtml;
    QString rightHtml;

    for (const auto& line : m_diffResult) {
        const QString bg = diffColor(line.type).name();
        const QString escapedLeft = line.leftText.toHtmlEscaped();
        const QString escapedRight = line.rightText.toHtmlEscaped();

        switch (line.type) {
        case DiffLineType::Unchanged:
            leftHtml += QStringLiteral("<div style=\"background:%1\">%2</div>").arg(bg, escapedLeft);
            rightHtml += QStringLiteral("<div style=\"background:%1\">%2</div>").arg(bg, escapedRight);
            break;
        case DiffLineType::Added:
            ++m_added;
            leftHtml += QStringLiteral("<div style=\"background:%1\">&nbsp;</div>").arg(bg);
            rightHtml += QStringLiteral("<div style=\"background:%1\">+%2</div>").arg(bg, escapedRight);
            break;
        case DiffLineType::Removed:
            ++m_removed;
            leftHtml += QStringLiteral("<div style=\"background:%1\">-%2</div>").arg(bg, escapedLeft);
            rightHtml += QStringLiteral("<div style=\"background:%1\">&nbsp;</div>").arg(bg);
            break;
        case DiffLineType::Modified:
            ++m_modified;
            leftHtml += QStringLiteral("<div style=\"background:%1\">~%2</div>").arg(bg, escapedLeft);
            rightHtml += QStringLiteral("<div style=\"background:%1\">~%2</div>").arg(bg, escapedRight);
            break;
        }
    }

    m_leftEdit->setHtml(leftHtml);
    m_rightEdit->setHtml(rightHtml);

    // 更新统计标签
    m_statsLabel->setText(
        tr("差异统计: +%1 新增  -%2 删除  ~%3 修改")
            .arg(m_added).arg(m_removed).arg(m_modified));
    m_totalDiffs += static_cast<quint64>(m_added + m_removed + m_modified);
}

// ============================================================================
// 颜色映射
// ============================================================================

QColor DataDiffWidget::diffColor(DiffLineType type) const
{
    auto& theme = ThemeManager::instance();
    switch (type) {
    case DiffLineType::Added:
        return theme.color(ThemeManager::SemanticColor::Success);
    case DiffLineType::Removed:
        return theme.color(ThemeManager::SemanticColor::Error);
    case DiffLineType::Modified:
        return theme.color(ThemeManager::SemanticColor::Warning);
    default:
        return QColor(0, 0, 0, 0); // 透明(使用默认背景)
    }
}

// ============================================================================
// 统计计数器
// ============================================================================

/** @brief 重置对比统计计数器(对比次数/差异数/对比字节数) */
void DataDiffWidget::resetDiffStatistics()
{
    m_totalComparisons = 0;
    m_totalDiffs = 0;
    m_totalBytesCompared = 0;
}
