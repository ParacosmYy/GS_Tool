/**
 * @file WalshHadamard.h
 * @brief 快速Walsh-Hadamard变换(序率/位反转排序) — Fast Walsh-Hadamard Transform with Sequency and Bit-Reversal Ordering
 *
 * 功能: 实现快速Walsh-Hadamard变换(FWHT)，支持自然序(Hadamard序)和
 *       序率序(Walsh序)排列，含位反转重排序。O(n log n)蝶形运算。
 *
 * 协作: ModifiedDCT(DCT变换) / DiscreteCosineTransform(DCT) / FftEngine(FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Walsh-Hadamard变换器
 */
class WalshHadamard : public QObject {
    Q_OBJECT

public:
    /** @brief 输出排序方式 */
    enum Ordering {
        Natural,    ///< 自然序(Hadamard序)
        Sequency,   ///< 序率序(Walsh序，按过零数排列)
        BitReversal ///< 位反转序(Dyadic序)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalForward = 0;          ///< 累计正变换次数
        quint64 totalInverse = 0;          ///< 累计逆变换次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        int lastTransformSize = 0;         ///< 最近变换长度
    };

    explicit WalshHadamard(QObject* parent = nullptr);
    ~WalshHadamard() override;

    /** @brief 设置输出排序方式 */
    void setOrdering(Ordering order);

    /**
     * @brief 正变换(FWHT)
     * @param input 输入序列(长度须为2的幂)
     * @return 变换系数
     */
    QVector<double> forward(const QVector<double>& input);

    /**
     * @brief 逆变换(IFWHT)，FWHT是自逆变换(只差1/N缩放)
     * @param coeffs 变换系数
     * @return 时域信号
     */
    QVector<double> inverse(const QVector<double>& coeffs);

    /**
     * @brief 将自然序转为序率序
     * @param data 自然序数据
     * @return 序率序数据
     */
    static QVector<double> toSequencyOrder(const QVector<double>& data);

    /**
     * @brief 将自然序转为位反转序
     * @param data 自然序数据
     * @return 位反转序数据
     */
    static QVector<double> toBitReversalOrder(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 正变换完成 @param size 变换长度 */
    void forwardCompleted(int size);
    /** @brief 逆变换完成 @param size 变换长度 */
    void inverseCompleted(int size);

private:
    /** @brief 核心FWHT蝶形运算(原地) */
    static void fwhtCore(QVector<double>& data);

    /** @brief 计算位反转索引 */
    static int bitReverse(int x, int log2n);

    /** @brief 计算Gray码 */
    static int grayCode(int x);

    Ordering m_ordering = Natural;

    Stats m_stats;
    double m_timeSum = 0.0;
};
