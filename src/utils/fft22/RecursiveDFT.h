/**
 * @file RecursiveDFT.h
 * @brief 递归DFT(Cooley-Tukey) — 任意基数/旋转因子/混合基分解
 *
 * 功能: 实现通用递归DFT，支持任意基数的Cooley-Tukey分解、
 *       旋转因子表预计算、混合基分解策略和Rader算法处理素数长度。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / FftEngine(FFT引擎)
 */
#ifndef RECURSIVEDFT_H
#define RECURSIVEDFT_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 递归DFT处理器 — Cooley-Tukey任意基数分解
 */
class RecursiveDFT : public QObject {
    Q_OBJECT

public:
    /** @brief 分解策略 */
    enum class Strategy {
        AutoRadix2,     ///< 自动选择基2
        MixedRadix,     ///< 混合基数分解
        RaderPrime      ///< Rader算法处理素数
    };
    Q_ENUM(Strategy)

    /** @brief 统计 */
    struct Stats {
        quint64 totalTransforms = 0;        ///< 累计变换次数
        quint64 totalPointsProcessed = 0;   ///< 累计处理点数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        int     maxRadixUsed = 2;           ///< 最大使用基数
    };

    explicit RecursiveDFT(QObject* parent = nullptr);

    /** @brief 设置分解策略 @param strategy 策略 */
    void setStrategy(Strategy strategy);

    /** @brief 执行前向DFT @param real 实部 @param imag 虚部 @return (实部,虚部) */
    QPair<QVector<double>, QVector<double>> forward(
        const QVector<double>& real,
        const QVector<double>& imag = {});

    /** @brief 执行逆DFT @param real 实部 @param imag 虚部 @return (实部,虚部) */
    QPair<QVector<double>, QVector<double>> inverse(
        const QVector<double>& real,
        const QVector<double>& imag);

    /** @brief 获取旋转因子表 @param n 长度 @return (cos表, sin表) */
    QPair<QVector<double>, QVector<double>> twiddleFactors(int n) const;

    /** @brief 因数分解 @param n 正整数 @return 因数列表 */
    QList<int> factorize(int n) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成 @param n 变换长度 @param forward 是否前向 */
    void transformComplete(int n, bool forward);

private:
    void recursiveCT(QVector<double>& re, QVector<double>& im,
                     int start, int stride, int n, bool inverse);
    void raderDFT(QVector<double>& re, QVector<double>& im,
                  int start, int stride, int n, bool inverse);
    int primitiveRoot(int p) const;
    int modPow(int base, int exp, int mod) const;
    int findBestRadix(int n) const;

    void buildTwiddleTable(int n);
    QVector<double> m_twiddleCos;   ///< 旋转因子余弦表
    QVector<double> m_twiddleSin;   ///< 旋转因子正弦表
    int m_twiddleSize;              ///< 旋转因子表大小

    Strategy m_strategy;            ///< 分解策略

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};

#endif // RECURSIVEDFT_H
