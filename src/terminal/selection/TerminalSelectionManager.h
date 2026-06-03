/**
 * @file TerminalSelectionManager.h
 * @brief 终端选区管理器 - 管理终端文本选择的鼠标交互和选区文本提取
 *
 * 从 TerminalWidget 中拆分出来，负责:
 *   - 鼠标按下/移动/释放时的选区状态维护
 *   - 正规化选区范围(支持反向拖选)
 *   - 从缓存行或方向过滤索引表中提取选中文本
 *   - 选区操作统计(选择次数、复制次数、字符数、最大选区)
 *
 * 协作关系:
 *   - TerminalWidget: 持有本类实例，在鼠标事件和绘制时委托调用
 *   - DirectionFilter: 选区文本提取时需要访问过滤索引表
 *   - CachedLine: 选区文本从缓存行数据中获取
 */

#ifndef TERMINALSELECTIONMANAGER_H
#define TERMINALSELECTIONMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QVector>
#include "terminal/types/TerminalTypes.h"

class DirectionFilter;

/**
 * @brief 终端选区管理器 - 管理鼠标文本选择交互
 *
 * 跟踪鼠标拖拽选择的起始/结束行号，提供正规化的选区范围和
 * 从缓存中提取选中文本的能力。支持方向过滤模式下的选区文本提取。
 * 同时维护选区操作的统计计数器，用于运行时分析。
 *
 * 设计模式: 组合模式 — TerminalWidget 通过组合持有本类，
 * 所有鼠标选择相关的状态和行为集中在此管理。
 */
class TerminalSelectionManager : public QObject {
    Q_OBJECT

public:
    explicit TerminalSelectionManager(QObject* parent = nullptr);

    /** @brief 处理鼠标按下事件 - 开始选择 @param y 鼠标Y坐标(像素) @param scrollOffset 当前滚动偏移(行数) @param lineHeight 每行像素高度 */
    void onMousePress(double y, int scrollOffset, int lineHeight);

    /** @brief 处理鼠标移动事件 - 扩展选择范围 @param y 鼠标Y坐标(像素) @param scrollOffset 当前滚动偏移(行数) @param lineHeight 每行像素高度 */
    void onMouseMove(double y, int scrollOffset, int lineHeight);

    /** @brief 处理鼠标释放事件 - 结束选择 */
    void onMouseRelease();

    /** @brief 获取选中的文本 @param cachedLines 缓存行数据 @param directionFilter 方向过滤器 @return 选中的文本，无选择时返回空字符串 */
    QString selectedText(const QVector<CachedLine>& cachedLines,
                         const DirectionFilter* directionFilter) const;

    /** @brief 重置选区状态(数据清空时调用) */
    void reset();

    /** @brief 获取正规化的选区起始行号 @return 较小的选区行号 */
    int normalizedStartLine() const;

    /** @brief 获取正规化的选区结束行号 @return 较大的选区行号 */
    int normalizedEndLine() const;

    /** @brief 当前是否有有效的选区 */
    bool hasSelection() const;

    /** @brief 获取选中背景色 */
    QColor selectionBgColor() const;

    /** @brief 设置选中背景色 */
    void setSelectionBgColor(const QColor& color);

    /** @brief 程序化设置选区范围（用于全选操作） @param startLine 起始行号 @param endLine 结束行号 */
    void setSelection(int startLine, int endLine);

    /** @brief 通知复制操作已完成，递增复制计数并更新字符数统计 @param charCount 本次复制的字符数 */
    void notifyCopyPerformed(quint64 charCount = 0);

    // ── 统计计数器 Getter ──

    /** @brief 获取总选择次数 @return 用户完成选区的总次数 */
    quint64 totalSelections() const;

    /** @brief 获取总复制次数 @return 用户执行复制操作的总次数 */
    quint64 totalCopies() const;

    /** @brief 获取总选择字符数 @return 历史所有选区字符数之和 */
    quint64 totalSelectionChars() const;

    /** @brief 获取最大单次选区长度(字符数) @return 历史最大选区的字符数 */
    quint64 maxSelectionLength() const;

    /** @brief 重置所有统计计数器为零 */
    void resetStats();

private:
    /** @brief 更新选区字符数统计(累计+最大选区) @param charCount 本次选区字符数 */
    void updateSelectionStats(quint64 charCount);

    int m_selectionStartLine = -1;  ///< 选区起始行号(可能是拖拽起点或终点)
    int m_selectionEndLine = -1;    ///< 选区结束行号(可能是拖拽起点或终点)
    bool m_isSelecting = false;     ///< 是否正在拖拽选择中

    QColor m_selectionBg;           ///< 选中区域背景色

    // ── 统计计数器 ──
    quint64 m_totalSelections = 0;      ///< 总选择次数(每次新选区递增)
    quint64 m_totalCopies = 0;          ///< 总复制次数(每次复制操作递增)
    quint64 m_totalSelectionChars = 0;  ///< 总选择字符数(所有选区字符数累加)
    quint64 m_maxSelectionLength = 0;   ///< 最大单次选区长度(字符数)
};

#endif // TERMINALSELECTIONMANAGER_H
