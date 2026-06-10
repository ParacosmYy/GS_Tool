/**
 * @file RegisterMapModel.h
 * @brief 寄存器地图表格模型 -- 为 QTableView 提供寄存器数据
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 实现 QAbstractTableModel，以 Address/Name/Value/Reset/Group 五列展示
 * 寄存器地图数据。Value 列可编辑，其余列为只读。
 * 统计查询方法见：@see RegisterMapModelStats.cpp
 */

#ifndef REGISTERMAPMODEL_H
#define REGISTERMAPMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include "utils/regmap/RegisterMapTypes.h"

/**
 * @class RegisterMapModel
 * @brief 寄存器地图的表格数据模型
 *
 * 管理 RegisterMap 数据并以标准 Qt Model/View 架构暴露给 QTableView。
 * Value 列支持内联编辑；编辑后发出 dataChanged 信号。
 */
class RegisterMapModel : public QAbstractTableModel {
    Q_OBJECT

public:
    /** @brief 表格列枚举 */
    enum Column {
        ColAddress = 0,  ///< 地址列
        ColName,         ///< 名称列
        ColValue,        ///< 当前值列（可编辑）
        ColReset,        ///< 复位值列
        ColGroup,        ///< 分组列
        ColCount         ///< 列总数
    };

    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalEdits = 0;       ///< 值编辑次数
        quint64 totalLoads = 0;       ///< 地图加载次数
        quint64 totalSearches = 0;    ///< 搜索次数
        quint64 totalExports = 0;     ///< 导出次数
        int peakRegisterCount = 0;    ///< 峰值寄存器数
        int activeRegisterCount = 0;  ///< 当前寄存器数
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit RegisterMapModel(QObject *parent = nullptr);

    /** @brief 加载寄存器地图 @param map 寄存器地图数据 */
    void loadRegisterMap(const RegisterMap &map);

    /** @brief 获取当前寄存器地图 @return 寄存器地图常引用 */
    const RegisterMap &registerMap() const;

    /** @brief 获取指定行寄存器 @param row 行号 @return 寄存器条目 */
    RegisterEntry registerAt(int row) const;

    /** @brief 获取指定行当前值 @param row 行号 @return 当前值 */
    quint64 valueAt(int row) const;

    /** @brief 设置指定行当前值 @param row 行号 @param val 新值 */
    void setValueAt(int row, quint64 val);

    /** @brief 按关键词过滤（名称/地址/分组） @param keyword 过滤关键词 */
    void setFilter(const QString &keyword);

    /** @brief 清除过滤，恢复全部显示 */
    void clearFilter();

    /** @brief 获取统计信息 @return 统计快照常引用 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

    // ---- QAbstractTableModel 接口 ----
    /** @copydoc QAbstractTableModel::rowCount */
    int rowCount(const QModelIndex &parent = {}) const override;

    /** @copydoc QAbstractTableModel::columnCount */
    int columnCount(const QModelIndex &parent = {}) const override;

    /** @copydoc QAbstractTableModel::data */
    QVariant data(const QModelIndex &index, int role) const override;

    /** @copydoc QAbstractTableModel::headerData */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    /** @copydoc QAbstractTableModel::flags */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /** @copydoc QAbstractTableModel::setData */
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

signals:
    /** @brief 寄存器值被修改信号 @param row 行号 @param newValue 新值 */
    void registerValueChanged(int row, quint64 newValue);

    /** @brief 寄存器地图加载完成信号 @param count 寄存器总数 */
    void registerMapLoaded(int count);

private:
    void rebuildFilteredIndices();  ///< 重建过滤索引

    RegisterMap m_map;                     ///< 原始寄存器地图
    QVector<int> m_filteredRows;           ///< 过滤后的行索引
    QVector<quint64> m_currentValues;      ///< 当前值数组
    QString m_filterKeyword;               ///< 过滤关键词
    Stats m_stats;                         ///< 统计计数器
};

#endif // REGISTERMAPMODEL_H
