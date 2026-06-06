/**
 * @file NumberTheoreticTransform.h
 * @brief 数论变换(有限域Cooley-Tukey蝶形运算) — Number Theoretic Transform over Finite Field with Cooley-Tukey Butterfly
 *
 * 功能: 实现有限域上的数论变换(NTT)，采用Cooley-Tukey蝶形运算，
 *       支持模素数域上的循环卷积，适用于大整数乘法与多项式运算。
 *
 * 协作: WalshHadamard(正交变换) / FftEngine(FFT) / DiscreteCosineTransform(DCT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 数论变换器
 */
class NumberTheoreticTransform : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalForward = 0;          ///< 累计正变换次数
        quint64 totalInverse = 0;          ///< 累计逆变换次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        int lastTransformSize = 0;         ///< 最近变换长度
    };

    explicit NumberTheoreticTransform(QObject* parent = nullptr);
    ~NumberTheoreticTransform() override;

    /** @brief 设置模数(须为形如 k*2^n+1 的素数) */
    void setModulus(quint64 mod);

    /** @brief 设置原根 */
    void setPrimitiveRoot(quint64 g);

    /**
     * @brief 正向NTT
     * @param input 输入序列(长度须为2的幂)
     * @return NTT系数(模运算结果)
     */
    QVector<quint64> forward(const QVector<quint64>& input);

    /**
     * @brief 逆向NTT(INTT)
     * @param coeffs NTT系数
     * @return 原域序列
     */
    QVector<quint64> inverse(const QVector<quint64>& coeffs);

    /**
     * @brief 有限域循环卷积(NTT方法)
     * @param a 序列A
     * @param b 序列B
     * @return 卷积结果
     */
    QVector<quint64> cyclicConvolution(const QVector<quint64>& a,
                                       const QVector<quint64>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 正变换完成 @param size 变换长度 */
    void forwardCompleted(int size);
    /** @brief 逆变换完成 @param size 变换长度 */
    void inverseCompleted(int size);

private:
    /** @brief 模加法 */
    quint64 modAdd(quint64 a, quint64 b) const;

    /** @brief 模减法 */
    quint64 modSub(quint64 a, quint64 b) const;

    /** @brief 模乘法(防止溢出) */
    quint64 modMul(quint64 a, quint64 b) const;

    /** @brief 快速模幂 */
    quint64 modPow(quint64 base, quint64 exp) const;

    /** @brief 模逆元 */
    quint64 modInverse(quint64 a) const;

    /** @brief 核心NTT蝶形运算 */
    void nttCore(QVector<quint64>& data, bool inverse);

    quint64 m_mod = 998244353;  ///< 默认模数(119*2^23+1)
    quint64 m_root = 3;        ///< 默认原根

    Stats m_stats;
    double m_timeSum = 0.0;
};
