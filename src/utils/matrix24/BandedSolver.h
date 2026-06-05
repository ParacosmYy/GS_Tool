/**
 * @file BandedSolver.h
 * @brief 带状矩阵求解器 — LU分解/Thomas三对角/紧致存储
 *
 * 功能: 求解带状线性方程组，支持LU分解(一般带状)、Thomas算法
 *       (三对角)、紧致带状存储格式，以及多右端项求解。
 *
 * 协作: DataInterpolator(插值) / DigitalFilter(滤波)
 */
#ifndef BANDEDSOLVER_H
#define BANDEDSOLVER_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 带状矩阵求解器
 */
class BandedSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 求解方法 */
    enum class Method {
        ThomasAlgorithm,    ///< Thomas追赶法(三对角)
        BandedLU            ///< 带状LU分解(一般带状)
    };
    Q_ENUM(Method)

    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        quint64 totalSystemsSize = 0;       ///< 累计方程组总大小
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        quint64 totalFactorizations = 0;    ///< 累计分解次数
    };

    explicit BandedSolver(QObject* parent = nullptr);

    /** @brief 设置三对角矩阵并求解 @param lower 下对角线(长度n-1) @param main 主对角线(长度n) @param upper 上对角线(长度n-1) @param rhs 右端向量(长度n) @return 解向量 */
    QVector<double> solveTridiagonal(const QVector<double>& lower,
                                     const QVector<double>& main,
                                     const QVector<double>& upper,
                                     const QVector<double>& rhs);

    /** @brief Thomas追赶法 @param a 下对角线 @param b 主对角线 @param c 上对角线 @param d 右端 @return 解 */
    QVector<double> thomasAlgorithm(const QVector<double>& a,
                                    const QVector<double>& b,
                                    const QVector<double>& c,
                                    const QVector<double>& d);

    /** @brief 设置带状矩阵(紧致存储) @param bands 带状存储(每行一条对角线) @param n 矩阵维度 @param lowerBand 下带宽 @param upperBand 上带宽 */
    void setBandedMatrix(const QVector<QVector<double>>& bands,
                         int n, int lowerBand, int upperBand);

    /** @brief LU分解带状矩阵 @return 是否成功(主元不为零) */
    bool factorize();

    /** @brief 求解单右端项 @param rhs 右端向量 @return 解向量 */
    QVector<double> solve(const QVector<double>& rhs);

    /** @brief 求解多右端项 @param rhsList 右端向量列表 @return 解向量列表 */
    QList<QVector<double>> solveMultiple(const QList<QVector<double>>& rhsList);

    /** @brief 获取LU分解后的矩阵 @return 紧致存储 */
    QVector<QVector<double>> factoredMatrix() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param n 方程组大小 @param numRhs 右端项数 */
    void solveComplete(int n, int numRhs);

    /** @brief 分解完成 @param n 矩阵大小 @param success 是否成功 */
    void factorizationComplete(int n, bool success);

private:
    int m_n;                        ///< 矩阵维度
    int m_lowerBand;                ///< 下带宽
    int m_upperBand;                ///< 上带宽
    QVector<QVector<double>> m_bands; ///< 紧致存储带状矩阵
    bool m_factored;                ///< 是否已分解

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};

#endif // BANDEDSOLVER_H
