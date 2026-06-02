/**
 * @file TerminalSearchRenderer.cpp
 * @brief 终端搜索高亮渲染器实现 - 绘制搜索匹配的高亮矩形
 *
 * 将 TerminalWidget::paintLine 中的搜索高亮绘制逻辑提取为独立函数，
 * 降低 paintLine 的复杂度，使 TerminalWidget.cpp 保持在500行以内。
 */

#include "terminal/search/TerminalSearchRenderer.h"
#include "terminal/search/TerminalSearchManager.h"

/** @brief 在终端绘制搜索高亮矩形(遍历匹配位置→计算字符偏移→填充高亮色)
 * @param painter QPainter引用
 * @param fontMetrics 字体度量(计算字符宽度)
 * @param searchManager 搜索管理器(提供匹配索引)
 * @param cached 当前行缓存(字符偏移列表)
 * @param displayLine 显示行号
 * @param textXOffset 文本X偏移
 * @param y 当前行Y坐标
 * @param lineHeight 行高
 */
void TerminalSearchRenderer::drawHighlights(
    QPainter& painter,
    const QFontMetrics& fontMetrics,
    const TerminalSearchManager* searchManager,
    const CachedLine& cached,
    int displayLine,
    int textXOffset,
    int y,
    int lineHeight,
    bool showDirectionPrefix)
{
    // 注意: HEX搜索时，lineProvider返回HexConverter::toHexString()格式的文本进行匹配，
    // 而cached.text可能是Text/Mixed/Decimal格式。高亮位置仅在Hex显示模式下准确。
    // 建议HEX搜索时自动切换到Hex显示模式以保证高亮对齐。
    const auto& matches = searchManager->searchMatches();
    if (matches.isEmpty()) return;

    int curIdx = searchManager->currentMatchIndex();

    // 方向前缀 "[TX:] " / "[RX:] " 的字符长度
    // match.startCol 是包含前缀的文本中的列偏移，需要减去前缀长度
    // 使高亮起始位置与实际显示的文本内容对齐，避免双倍偏移
    static const QString kTxPrefix = QStringLiteral("[TX:] ");
    static const QString kRxPrefix = QStringLiteral("[RX:] ");

    int prefixLen = 0;
    if (showDirectionPrefix) {
        const QString& prefix = (cached.direction == DataDirection::Tx) ? kTxPrefix : kRxPrefix;
        if (cached.text.startsWith(prefix)) {
            prefixLen = prefix.length();
        }
    }

    // 遍历所有匹配项，筛选属于当前行的匹配并绘制高亮矩形
    for (int mi = 0; mi < matches.size(); ++mi) {
        const auto& match = matches[mi];
        if (match.line != displayLine) continue;

        // match.startCol 是包含前缀的文本中的列偏移，减去前缀长度后
        // 得到在实际显示文本内容中的列位置
        int col = match.startCol - prefixLen;
        if (col < 0) col = 0;

        QString textForWidth = cached.text.mid(prefixLen);
        int xStart = textXOffset + fontMetrics.horizontalAdvance(textForWidth.left(col));
        int matchWidth = fontMetrics.horizontalAdvance(textForWidth.mid(col, match.length));

        // 当前匹配使用高亮强调色，其他匹配使用普通高亮色
        painter.fillRect(xStart, y + 2, matchWidth, lineHeight - 4,
                         (mi == curIdx) ? searchManager->currentMatchColor()
                                        : searchManager->searchHighlightColor());
    }
}
