/**
 * @file SturmSequence.h
 * @brief Sturm序列 — 三对角矩阵特征值计数与隔离
 *
 * 功能: 构造Sturm序列计算三对角矩阵在给定阈值以下的特征值个数，
 *       用于特征值二分搜索和区间隔离。
 *
 * 协作: QrEigenSolver(QR特征值) / PowerIteration(幂迭代)
 */
#ifndef STURMSEQUENCE_H
#define STURMSEQUENCE_H

#include <QObject>
#include <QVector>

/**
 * @brief Sturm序列特征值计数器
 */
class SturmSequence : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalComputations = 0;      ///< 累计计算次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit SturmSequence(QObject* parent = nullptr);

    /**
     * @brief 计算小于x的特征值个数
     * @param diag 三对角矩阵主对角线
     * @param offDiag 次对角线
     * @param x 阈值
     * @return 小于x的特征值个数
     */
    int countEigenvalues(const QVector<double>& diag,
                         const QVector<double>& offDiag, double x) const;

    /**
     * @brief 隔离全部特征值
     * @param diag 主对角线
     * @param offDiag 次对角线
     * @param lower 搜索下界
     * @param upper 搜索上界
     * @param nEigenvalues 特征值个数
     * @return 特征值向量(升序)
     */
    QVector<double> isolateEigenvalues(const QVector<double>& diag,
                                        const QVector<double>& offDiag,
                                        double lower, double upper,
                                        int nEigenvalues);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成 @param eigenvalueCount 特征值个数 */
    void computationCompleted(int eigenvalueCount);

private:
    Stats m_stats;              ///< 统计信息
    double m_timeSum = 0.0;     ///< 累计耗时
};

#endif // STURMSEQUENCE_H
