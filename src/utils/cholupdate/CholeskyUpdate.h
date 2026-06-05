/**
 * @file CholeskyUpdate.h
 * @brief Cholesky分解秩-1更新/降级
 *
 * 功能: 对已有的Cholesky分解L执行秩-1更新(L*L^T + x*x^T)
 *       和秩-1降级(L*L^T - x*x^T)。基于Givens旋转实现，
 *       无需从头重新分解。统计更新次数和平均耗时。
 */
#ifndef CHOLESKYUPDATE_H
#define CHOLESKYUPDATE_H

#include <QObject>
#include <QVector>

/**
 * @class CholeskyUpdate
 * @brief Cholesky分解秩-1更新/降级工具类
 */
class CholeskyUpdate : public QObject {
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
    explicit CholeskyUpdate(QObject* parent = nullptr);

    /**
     * @brief Cholesky秩-1更新: L*L^T + x*x^T
     * @param L 下三角Cholesky因子(会被就地修改)
     * @param x 更新向量
     * @return 更新是否成功(数值稳定时返回true)
     */
    bool update(QVector<QVector<double>>& L,
                const QVector<double>& x);

    /**
     * @brief Cholesky秩-1降级: L*L^T - x*x^T
     * @param L 下三角Cholesky因子(会被就地修改)
     * @param x 降级向量
     * @return 降级是否成功(结果必须正定)
     */
    bool downdate(QVector<QVector<double>>& L,
                  const QVector<double>& x);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 更新完成信号 @param size 矩阵维度 */
    void updateCompleted(int size);

private:
    Stats  m_stats;   /**< 统计数据 */
    double m_timeSum; /**< 累计耗时(ms) */
};

#endif // CHOLESKYUPDATE_H
