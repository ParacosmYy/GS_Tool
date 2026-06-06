/**
 * @file DiscreteHartleyTransform.h
 * @brief 离散哈特利变换(Cas2Cas蝶形+自逆性质) — Discrete Hartley Transform via Cas2Cas Butterfly with Self-Inverse Property
 *
 * 功能: 实现离散哈特利变换(DHT)，基于Cas2Cas蝶形运算，
 *       利用DHT自逆性质(正变换=逆变换)，支持卷积和相关计算。
 *
 * 协作: NumberTheoreticTransform(数论变换) / FftEngine(FFT) / WalshHadamard(正交变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散哈特利变换器
 */
class DiscreteHartleyTransform : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalForward = 0;          ///< 累计正变换次数
        quint64 totalInverse = 0;          ///< 累计逆变换次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        int lastTransformSize = 0;         ///< 最近变换长度
    };

    explicit DiscreteHartleyTransform(QObject *parent = nullptr);
    ~DiscreteHartleyTransform() override;

    /**
     * @brief 正向DHT
     * @param input 输入序列(长度须为2的幂)
     * @return DHT系数
     */
    QVector<double> forward(const QVector<double>& input);

    /**
     * @brief 逆向DHT(利用自逆性质: 再做一次DHT然后除以N)
     * @param coeffs DHT系数
     * @return 原域序列
     */
    QVector<double> inverse(const QVector<double>& coeffs);

    /**
     * @brief 通过DHT计算循环卷积
     * @param a 序列A
     * @param b 序列B
     * @return 卷积结果
     */
    QVector<double> cyclicConvolution(const QVector<double>& a,
                                      const QVector<double>& b);

    /**
     * @brief 通过DHT计算互相关
     * @param a 序列A
     * @param b 序列B
     * @return 相关结果
     */
    QVector<double> crossCorrelation(const QVector<double>& a,
                                     const QVector<double>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 正变换完成 @param size 变换长度 */
    void forwardCompleted(int size);
    /** @brief 逆变换完成 @param size 变换长度 */
    void inverseCompleted(int size);

private:
    /** @brief 核心蝶形DHT运算 */
    void dhtCore(QVector<double>& data) const;

    /** @brief Cas函数: cos(x) + sin(x) */
    static double cas(double x);

    Stats m_stats;
    double m_timeSum = 0.0;
};
