/**
 * @file QrUpdate.h
 * @brief QR分解更新 — 秩-1更新与列增删
 *
 * 功能: 对已有的QR分解执行秩-1更新(A + uv^T)、列添加操作，
 *       避免从头重新分解。基于Givens旋转实现增量更新。
 *       统计更新次数和平均耗时。
 */
#ifndef QRUPDATE_H
#define QRUPDATE_H

#include <QObject>
#include <QVector>

/**
 * @class QrUpdate
 * @brief QR分解增量更新工具类
 */
class QrUpdate : public QObject {
    Q_OBJECT
public:
    /** 统计信息结构体 */
    struct Stats {
        quint64 totalUpdates = 0;       /**< 总更新次数 */
        double  avgProcessingTimeMs = 0.0; /**< 平均处理耗时(ms) */
    };

    /**
     * @brief 构造函数
     * @param parent 父QObject
     */
    explicit QrUpdate(QObject* parent = nullptr);

    /**
     * @brief 秩-1更新 QR分解: A + u*v^T 的QR分解
     * @param Q 正交矩阵(会被就地修改)
     * @param R 上三角矩阵(会被就地修改)
     * @param u 列向量
     * @param v 行向量
     * @return 更新是否成功
     */
    bool rankOneUpdate(QVector<QVector<double>>& Q,
                       QVector<QVector<double>>& R,
                       const QVector<double>& u,
                       const QVector<double>& v);

    /**
     * @brief 向QR分解中添加一列
     * @param Q 正交矩阵(会被就地修改)
     * @param R 上三角矩阵(会被就地修改)
     * @param newCol 新增列向量
     * @return 添加是否成功
     */
    bool addColumn(QVector<QVector<double>>& Q,
                   QVector<QVector<double>>& R,
                   const QVector<double>& newCol);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 更新完成信号 @param size 矩阵维度 */
    void updateCompleted(int size);

private:
    /** @brief 应用单次Givens旋转到Q和R */
    void applyGivens(QVector<QVector<double>>& Q,
                     QVector<QVector<double>>& R,
                     int row1, int row2, int colStart, int colEnd);

    Stats  m_stats;   /**< 统计数据 */
    double m_timeSum; /**< 累计耗时(ms) */
};

#endif // QRUPDATE_H
