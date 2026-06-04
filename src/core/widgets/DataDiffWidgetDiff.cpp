/**
 * @file DataDiffWidgetDiff.cpp
 * @brief 数据对比视图 - LCS差异算法与显示刷新实现
 *
 * 从 DataDiffWidget.cpp 拆分而来，包含LCS差异计算、
 * HTML渲染刷新、颜色映射和统计重置方法。
 */

#include "core/widgets/DataDiffWidget.h"
#include "core/theme/ThemeManager.h"

#include <QTextEdit>
#include <QLabel>
#include <algorithm>

/** @brief 使用LCS(最长公共子序列)算法计算行级差异 @param left 左侧行列表 @param right 右侧行列表 @return 差异行向量，包含每行的类型和内容 */
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

/** @brief 根据差异结果刷新左右两侧HTML显示，并更新统计标签 */
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

/** @brief 根据差异行类型获取对应的高亮颜色 @param type 差异行类型(Added/Removed/Modified/Unchanged) @return 对应的QColor颜色 */
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

/** @brief 重置对比统计计数器(对比次数/差异数/对比字节数) */
void DataDiffWidget::resetDiffStatistics()
{
    m_totalComparisons = 0;
    m_totalDiffs = 0;
    m_totalBytesCompared = 0;
}
