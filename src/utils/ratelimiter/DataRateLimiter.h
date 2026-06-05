/**
 * @file DataRateLimiter.h
 * @brief 数据速率限制器 — 控制数据流发送/接收速率
 *
 * 功能: 令牌桶算法实现数据速率限制，支持突发流量、
 *       滑动窗口计数、速率告警。用于防止串口数据溢出。
 *
 * 协作: IConnection(send限速) / DataPipeline(流控)
 */
#ifndef DATARATELIMITER_H
#define DATARATELIMITER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QQueue>

/**
 * @brief 数据速率限制器 — 令牌桶算法限速
 */
class DataRateLimiter : public QObject {
    Q_OBJECT

public:
    /** @brief 限速策略 */
    enum class LimitPolicy {
        TokenBucket,    ///< 令牌桶(允许突发)
        LeakyBucket,    ///< 漏桶(匀速输出)
        SlidingWindow   ///< 滑动窗口计数
    };
    Q_ENUM(LimitPolicy)

    /** @brief 统计 */
    struct Stats {
        quint64 totalBytesInput = 0;    ///< 累计输入字节
        quint64 totalBytesOutput = 0;   ///< 累计输出字节
        quint64 totalBytesDropped = 0;  ///< 累计丢弃字节
        quint64 totalPacketsInput = 0;  ///< 累计输入包数
        quint64 totalPacketsDropped = 0;///< 累计丢弃包数
        quint64 totalTokenRefills = 0;  ///< 累计令牌补充次数
        double  peakInputRate = 0.0;    ///< 峰值输入速率(B/s)
        double  peakOutputRate = 0.0;   ///< 峰值输出速率(B/s)
        int     burstCount = 0;         ///< 突发次数
    };

    explicit DataRateLimiter(QObject* parent = nullptr);

    /** @brief 设置限速策略 @param policy 策略 */
    void setPolicy(LimitPolicy policy);

    /** @brief 设置目标速率 @param bytesPerSec 目标速率(字节/秒) */
    void setTargetRate(double bytesPerSec);

    /** @brief 设置桶容量(令牌桶/突发大小) @param capacity 容量(字节) */
    void setBucketCapacity(int capacity);

    /** @brief 尝试发送数据 @param data 待发送数据 @return 实际发送的数据(可能截断) */
    QByteArray trySend(const QByteArray& data);

    /** @brief 检查是否可以发送指定大小 @param bytes 字节数 @return 是否允许 */
    bool canSend(int bytes) const;

    /** @brief 获取当前可用令牌/配额 @return 可用字节数 */
    double availableQuota() const;

    /** @brief 获取当前速率 @return 当前输出速率(B/s) */
    double currentOutputRate() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 数据被限速丢弃 @param bytes 丢弃字节数 */
    void dataDropped(int bytes);
    /** @brief 速率超限告警 @param currentRate 当前速率 @param targetRate 目标速率 */
    void rateExceeded(double currentRate, double targetRate);

private slots:
    void onRefillTimer();

private:
    void refillTokens();

    QTimer m_refillTimer;          ///< 令牌补充定时器(100ms)
    QElapsedTimer m_elapsedTimer;  ///< 运行计时器

    LimitPolicy m_policy;          ///< 限速策略
    double m_targetRate;           ///< 目标速率(B/s)
    int m_bucketCapacity;          ///< 桶容量
    double m_currentTokens;        ///< 当前令牌数

    QQueue<QPair<qint64, int>> m_windowHistory; ///< 滑动窗口历史
    double m_lastOutputRate;       ///< 上次测量输出速率

    Stats m_stats;
};

#endif // DATARATELIMITER_H
