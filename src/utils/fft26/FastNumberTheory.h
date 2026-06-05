/**
 * @file FastNumberTheory.h
 * @brief 快速数论变换(NTT) — 模运算域FFT
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 快速数论变换(NTT)引擎
 * 在有限域上进行FFT,避免浮点误差
 */
class FastNumberTheory : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalTransforms = 0;
        int totalConvolutions = 0;
        int totalSamplesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FastNumberTheory(QObject* parent = nullptr);

    /** @brief 正向NTT @param data 输入数据 @param mod 模数 @param primRoot 原根 */
    QVector<long long> forward(const QVector<long long>& data,
                               long long mod = 998244353,
                               long long primRoot = 3);

    /** @brief 逆向NTT */
    QVector<long long> inverse(const QVector<long long>& data,
                                long long mod = 998244353,
                                long long primRoot = 3);

    /** @brief NTT域多项式乘法 @return 卷积结果 */
    QVector<long long> multiply(const QVector<long long>& a,
                                const QVector<long long>& b,
                                long long mod = 998244353);

    /** @brief 多项式求逆 mod x^n */
    QVector<long long> polyInverse(const QVector<long long>& a, int n,
                                    long long mod = 998244353);

    /** @brief 模幂运算 @return base^exp mod m */
    static long long modPow(long long base, long long exp, long long mod);

    /** @brief 模逆元 @return a^{-1} mod m */
    static long long modInverse(long long a, long long mod);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size);
    void convolutionCompleted(int resultSize);

private:
    void nttInPlace(QVector<long long>& data, bool inverse,
                    long long mod, long long primRoot) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
