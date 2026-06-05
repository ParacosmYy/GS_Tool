/**
 * @file SchurReorder.h
 * @brief Schur形式重排 — 将选定特征值聚集到对角线顶部
 *
 * 功能: 对实Schur形式T和正交矩阵Q，将selected指定的特征值
 *       通过隐式QR步交换到对角块左上角。用于特征值聚类、
 *       谱分割等应用。统计重排次数和平均耗时。
 */
#ifndef SCHURREORDER_H
#define SCHURREORDER_H

#include <QObject>
#include <QPair>
#include <QVector>

/**
 * @class SchurReorder
 * @brief Schur形式重排工具类
 */
class SchurReorder : public QObject {
    Q_OBJECT
public:
    /** 统计信息结构体 */
    struct Stats {
        quint64 totalReorders = 0;      /**< 总重排次数 */
        double  avgProcessingTimeMs = 0.0; /**< 平均处理耗时(ms) */
    };

    /**
     * @brief 构造函数
     * @param parent 父QObject
     */
    explicit SchurReorder(QObject* parent = nullptr);

    /**
     * @brief 重排Schur形式，将选定特征值聚集到左上角
     * @param T 实Schur上拟三角矩阵
     * @param Q Schur变换的正交矩阵
     * @param selected 需要聚集的特征值索引列表
     * @return QPair{重排后的T, 重排后的Q}
     */
    QPair<QVector<QVector<double>>, QVector<QVector<double>>>
    reorder(const QVector<QVector<double>>& T,
            const QVector<QVector<double>>& Q,
            const QVector<int>& selected);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 重排完成信号 @param size 矩阵维度 */
    void reorderCompleted(int size);

private:
    /**
     * @brief 交换Schur对角块p和q位置 (相邻交换)
     * @param T Schur矩阵(就地修改)
     * @param Q 正交矩阵(就地修改)
     * @param p 第一个块位置
     * @param q 第二个块位置(q = p+1 或 p+2)
     */
    void swapBlocks(QVector<QVector<double>>& T,
                    QVector<QVector<double>>& Q,
                    int p, int q);

    /** @brief 判断位置i处是否为2x2块 */
    bool isTwoByTwoBlock(const QVector<QVector<double>>& T, int i) const;

    Stats  m_stats;   /**< 统计数据 */
    double m_timeSum; /**< 累计耗时(ms) */
};

#endif // SCHURREORDER_H
