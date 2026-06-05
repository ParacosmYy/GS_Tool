/**
 * @file SturmSequence.h
 * @brief Sturm序列 — 多项式实根隔离
 *
 * 功能: 构造Sturm序列并通过符号变化计数隔离多项式在给定区间内的
 *       所有实根。用于多项式求根的预处理步骤。
 *
 * 协作: NewtonRaphson(根精化) / HornerScheme(多项式求值)
 */
#ifndef STURMSEQUENCE2_H
#define STURMSEQUENCE2_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Sturm序列多项式根隔离器
 *
 * Sturm定理: 多项式序列p0=p, p1=p', pi=-rem(pi-2,pi-1)
 * 在区间[a,b]内的符号变化数之差等于该区间内实根个数。
 */
class SturmChain : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalIsolated = 0;         ///< 累计隔离次数
        quint64 totalRoots = 0;            ///< 累计隔离的根总数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SturmChain(QObject* parent = nullptr);

    /**
     * @brief 计算多项式在区间[a,b]内的实根个数
     * @param coeffs 多项式系数(从高次到低次)
     * @param a 区间左端点
     * @param b 区间右端点
     * @return 实根个数
     */
    int countRoots(const QVector<double>& coeffs, double a, double b);

    /**
     * @brief 隔离多项式在[lo,hi]内所有实根的区间
     * @param coeffs 多项式系数(从高次到低次)
     * @param lo 搜索下界
     * @param hi 搜索上界
     * @return 各根的隔离区间列表
     */
    QVector<QPair<double, double>> isolateRoots(
        const QVector<double>& coeffs, double lo, double hi);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 根隔离完成信号 @param rootCount 隔离到的根数量 */
    void isolated(int rootCount);

private:
    /**
     * @brief 构造Sturm序列
     * @param coeffs 多项式系数
     * @return Sturm序列(多项式列表)
     */
    static QVector<QVector<double>> buildSequence(
        const QVector<double>& coeffs);

    /**
     * @brief 多项式除法取余式
     * @param dividend 被除式系数
     * @param divisor 除式系数
     * @return 余式系数(取负)
     */
    static QVector<double> polyRemainder(
        const QVector<double>& dividend,
        const QVector<double>& divisor);

    /**
     * @brief 计算多项式在x处的值
     * @param coeffs 多项式系数
     * @param x 求值点
     * @return 多项式值
     */
    static double polyEval(const QVector<double>& coeffs, double x);

    /**
     * @brief 计算Sturm序列在x处的符号变化数
     * @param sequence Sturm序列
     * @param x 求值点
     * @return 符号变化次数
     */
    static int signChanges(const QVector<QVector<double>>& sequence,
                           double x);

    Stats m_stats;              ///< 统计信息
    double m_timeSumMs = 0.0;   ///< 累计耗时(ms)
};

#endif // STURMSEQUENCE2_H
