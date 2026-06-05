/**
 * @file TerminalSearchRenderer.h
 * @brief 终端搜索高亮渲染器 - 负责绘制搜索匹配高亮矩形
 *
 * 从 TerminalWidget::paintLine 中提取出来的渲染辅助类，
 * 专职处理搜索匹配结果的视觉高亮绘制。
 *
 * 职责:
 *   - 遍历当前搜索匹配列表，在对应位置绘制高亮矩形
 *   - 区分"普通匹配"和"当前匹配"，使用不同颜色高亮
 *   - 处理方向前缀的列偏移对齐(高亮与实际文本内容对齐)
 *   - 利用matches按行排序特性，提前终止不匹配的行(优化)
 *
 * 协作关系:
 *   - TerminalSearchManager: 提供搜索匹配数据和颜色配置
 *   - TerminalWidget: 在 paintLine 中调用本类的 drawHighlights
 *
 * 设计模式: 工具类模式 — 无状态、无实例化需求，所有方法均为静态。
 * 统计: 渲染次数/高亮次数/当前高亮/空跳过 (静态计数器)
 */

#ifndef TERMINALSEARCHRENDERER_H
#define TERMINALSEARCHRENDERER_H

#include <QPainter>
#include <QFontMetrics>
#include <QtGlobal>
#include "terminal/types/TerminalTypes.h"

class TerminalSearchManager;

/**
 * @brief 终端搜索高亮渲染器 - 绘制搜索匹配的高亮矩形
 *
 * 本类将 paintLine 中约30行搜索高亮绘制逻辑封装为一个静态方法，
 * 使 paintLine 方法保持在80行以内，同时 TerminalWidget.cpp 降至500行以下。
 *
 * 使用方式:
 *   TerminalSearchRenderer::drawHighlights(painter, ...);
 *
 * 设计模式: 工具类模式 — 无状态、无实例化需求，所有方法均为静态。
 */
class TerminalSearchRenderer {
public:
    /**
     * @brief 在指定行绘制搜索匹配高亮矩形
     *
     * 遍历 searchManager 中的所有匹配项，筛选属于当前 displayLine 的匹配，
     * 在 QPainter 上绘制填充矩形。当前匹配使用高亮色，其他匹配使用普通色。
     *
     * @param painter      绘图上下文(已设置字体)
     * @param fontMetrics  字体度量(用于计算文本像素宽度)
     * @param searchManager 搜索管理器(提供匹配数据和颜色)
     * @param cached       当前行的缓存数据(用于获取文本和方向)
     * @param displayLine  当前显示行号(用于匹配过滤)
     * @param textXOffset  文本内容起始X偏移(跳过时间戳后的位置)
     * @param y            当前行的Y坐标(绘制高亮的顶部位置)
     * @param lineHeight   行高(绘制高亮的高度)
     * @param showDirectionPrefix 是否显示方向前缀(影响列偏移计算)
     */
    static void drawHighlights(QPainter& painter,
                               const QFontMetrics& fontMetrics,
                               const TerminalSearchManager* searchManager,
                               const CachedLine& cached,
                               int displayLine,
                               int textXOffset,
                               int y,
                               int lineHeight,
                               bool showDirectionPrefix);

    /** @brief 获取渲染总调用次数 */
    static quint64 totalRenders() { return s_totalRenders; }

    /** @brief 获取高亮匹配总次数 */
    static quint64 totalHighlights() { return s_totalHighlights; }

    /** @brief 获取当前匹配高亮总次数(当前选中匹配项的高亮绘制次数) */
    static quint64 totalCurrentHighlights() { return s_totalCurrentHighlights; }

    /** @brief 获取无匹配跳过总次数(搜索匹配为空时直接返回的次数) */
    static quint64 totalEmptySkips() { return s_totalEmptySkips; }

    /** @brief 重置所有统计计数器 */
    static void resetStatistics();

private:
    static quint64 s_totalRenders;           ///< 渲染总调用次数
    static quint64 s_totalHighlights;        ///< 高亮匹配总次数
    static quint64 s_totalCurrentHighlights; ///< 当前匹配高亮总次数
    static quint64 s_totalEmptySkips;        ///< 无匹配跳过总次数
};

#endif // TERMINALSEARCHRENDERER_H
