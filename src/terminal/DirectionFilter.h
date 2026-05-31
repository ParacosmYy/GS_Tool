/**
 * @file DirectionFilter.h
 * @brief 终端方向过滤器 - 根据数据方向(RX/TX)过滤终端显示行
 *
 * 分栏模式下，RX终端只显示接收数据行，TX终端只显示发送数据行。
 * 维护一个过滤索引表(m_filteredIndices)，在模型数据追加时增量更新。
 * 属于数据层(Data Layer)，不依赖任何UI组件。
 */

#ifndef DIRECTIONFILTER_H
#define DIRECTIONFILTER_H

#include <QObject>
#include <QVector>
#include <functional>
#include "core/Constants.h"
#include "terminal/TerminalTypes.h"

/**
 * @brief 终端方向过滤器 - 根据数据方向(RX/TX)过滤终端显示行
 *
 * 分栏模式下，RX终端只显示接收数据行，TX终端只显示发送数据行。
 * 维护一个过滤索引表(m_filteredIndices)，在模型数据追加时增量更新。
 *
 * 设计要点:
 *   - 属于数据层(Data Layer)，不依赖任何UI组件
 *   - 通过回调函数获取行数据，避免直接依赖TerminalModel
 *   - 增量更新机制: 只处理新增行，环形缓冲区回绕时全量重建
 *
 * 协作关系:
 *   - TerminalWidget: 持有并调用DirectionFilter进行方向过滤
 *   - TerminalModel: 通过回调间接访问，不直接依赖
 */
class DirectionFilter : public QObject {
    Q_OBJECT

public:
    explicit DirectionFilter(QObject* parent = nullptr);

    /**
     * @brief 设置过滤方向
     * @param direction 过滤方向(Rx=只显示接收, Tx=只显示发送)
     *
     * 设置后需要调用 reset() 或重新增量更新来重建索引表。
     */
    void setDirection(DataDirection direction);

    /** @brief 获取当前过滤方向 */
    DataDirection direction() const;

    /** @brief 清除过滤 - 显示全部数据行 */
    void clearFilter();

    /** @brief 是否启用了方向过滤 */
    bool isFiltered() const;

    /**
     * @brief 增量更新过滤索引表
     * @param modelLineCount 模型当前总行数
     * @param lineAt 回调函数: 通过模型行号获取TerminalLine数据
     *
     * 当模型有新数据追加时调用。只处理新增的模型行，将匹配方向的行号
     * 追加到过滤索引表。环形缓冲区回绕(行数减少)时自动全量重建。
     */
    void onDataAppended(int modelLineCount,
                        const std::function<TerminalLine(int)>& lineAt);

    /** @brief 过滤后的总行数(= m_filteredIndices.size()) */
    int filteredLineCount() const;

    /**
     * @brief 将过滤索引转换为模型行号
     * @param filteredIndex 过滤索引表中的位置 [0, filteredLineCount())
     * @return 对应的模型行号
     */
    int modelIndex(int filteredIndex) const;

    /** @brief 重置所有过滤状态，清空索引表和已跟踪的模型行数 */
    void reset();

private:
    DataDirection m_direction = DataDirection::Rx;  ///< 过滤方向(仅当m_filtered为true时有效)
    bool m_filtered = false;                        ///< 是否启用方向过滤
    int m_trackedLineCount = 0;                     ///< 已处理的模型行数(用于增量更新)

    /** @brief 过滤索引表: m_filteredIndices[显示行号] = 模型行号 */
    QVector<int> m_filteredIndices;
};

#endif // DIRECTIONFILTER_H
