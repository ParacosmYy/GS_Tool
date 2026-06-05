/**
 * @file DataRateLimiter.h
 * @brief 数据速率限制器 — 四种算法实现串口发送限速，防止缓冲区溢出
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供令牌桶、漏桶、滑动窗口、固定窗口四种限速算法，
 * 支持 canSend/acquire/waitTime 流控接口，完整的统计追踪。
 *
 * 协作: IConnection(send限速) / DataPipeline(流控) / SendController(发送节拍)
 */

#ifndef DATARATELIMITER_H
#define DATARATELIMITER_H

#include <QElapsedTimer>
#include <QQueue>
#include <QObject>
#include <QTimer>

/**
 * @class DataRateLimiter
 * @brief 数据速率限制器 — 多算法限速，防止串口缓冲区溢出
 *
 * 核心设计：
 * - 令牌桶(TokenBucket): 令牌随时间累积，消耗后发送，允许突发
 * - 漏桶(LeakyBucket): 恒定速率输出，无视突发
 * - 滑动窗口(SlidingWindow): 滑动时间窗口内计数
 * - 固定窗口(FixedWindow): 固定时间间隔内计数，简单高效
 * - 流控三件套: canSend()检查 / acquire()消耗 / waitTime()预估
 * - 统计覆盖: 通过/拒绝/等待/峰值/分算法拒绝计数
 */
class DataRateLimiter : public QObject {
    Q_OBJECT

public:
    /** @brief 限速算法枚举 */
    enum class Algorithm {
        TokenBucket,   ///< 令牌桶: 令牌累积，按需消耗，允许突发
        LeakyBucket,   ///< 漏桶: 恒定速率输出，无视突发流量
        SlidingWindow, ///< 滑动窗口: 在滑动时间窗口内统计字节数
        FixedWindow    ///< 固定窗口: 在固定时间间隔内统计字节数
    };
    Q_ENUM(Algorithm)

    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalBytesAllowed       = 0;  ///< 累计允许通过的字节总数
        quint64 totalBytesRejected      = 0;  ///< 累计拒绝的字节总数
        quint64 totalWaits              = 0;  ///< 累计等待次数
        double  avgWaitTimeMs           = 0.0;///< 平均等待时间(ms)
        double  peakRate                = 0.0;///< 峰值速率(Bytes/s)
        double  currentRate             = 0.0;///< 当前实时速率(Bytes/s)
        quint64 rejectionsByAlgorithm[4] = {};///< 按算法分类的拒绝计数 [TokenBucket,LeakyBucket,SlidingWindow,FixedWindow]
    };

    /**
     * @brief 构造数据速率限制器
     * @param parent 父对象
     */
    explicit DataRateLimiter(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~DataRateLimiter() override;

    // ── 算法选择 ──

    /** @brief 设置限速算法 @param algo 目标算法 */
    void setAlgorithm(Algorithm algo);

    /** @brief 获取当前算法 @return 当前算法枚举 */
    Algorithm algorithm() const { return m_algorithm; }

    // ── 速率配置 ──

    /** @brief 设置目标速率 @param bytesPerSecond 每秒允许的字节数 */
    void setRate(quint64 bytesPerSecond);

    /** @brief 设置突发大小(桶容量) @param maxBurst 最大突发字节数 */
    void setBurstSize(quint64 maxBurst);

    // ── 流控接口 ──

    /**
     * @brief 检查当前是否允许发送指定字节数(不消耗配额)
     * @param byteCount 目标字节数
     * @return true 表示限速允许发送
     */
    bool canSend(int byteCount) const;

    /**
     * @brief 尝试获取发送配额(消耗令牌/信用)
     * @param byteCount 目标字节数
     * @return true 表示成功获取配额
     */
    bool acquire(int byteCount);

    /**
     * @brief 计算发送指定字节数前需要等待的时间
     * @param byteCount 目标字节数
     * @return 预估等待时间(ms)，0 表示无需等待
     */
    int waitTime(int byteCount) const;

    /** @brief 重置限速器状态(配额/窗口/令牌恢复满) */
    void reset();

    // ── 统计接口 ──

    /** @brief 获取统计快照 @return Stats 结构体 */
    Stats stats() const;

    /** @brief 重置所有统计计数器(不影响当前配额状态) */
    void resetStatistics();

signals:
    /**
     * @brief 数据被限速拒绝信号
     * @param bytesRejected 被拒绝的字节数
     */
    void rateLimited(int bytesRejected);

    /**
     * @brief 等待请求信号(发送需等待时发射)
     * @param ms 预估等待毫秒数
     */
    void waitRequested(int ms);

private slots:
    /** @brief 内部定时器回调: 补充令牌/更新速率/切换窗口 */
    void onTick();

private:
    /** @brief 补充令牌桶令牌 */
    void refillTokens();

    /** @brief 更新实时速率统计 */
    void updateRateStats();

    /** @brief 获取滑动窗口已用字节数 @return 窗口内已发送字节总数 */
    quint64 slidingWindowUsed() const;

    /** @brief 获取固定窗口已用字节数 @return 当前窗口内已发送字节总数 */
    quint64 fixedWindowUsed() const;

    // ── 定时器 ──
    QTimer        m_tickTimer;          ///< 100ms 定时器，补充令牌/更新速率
    QElapsedTimer m_elapsed;            ///< 运行计时器

    // ── 配置 ──
    Algorithm     m_algorithm = Algorithm::TokenBucket; ///< 当前限速算法
    quint64       m_rate      = 1024;   ///< 目标速率(Bytes/s)
    quint64       m_burstSize = 2048;   ///< 桶容量/突发上限(Bytes)

    // ── 令牌桶状态 ──
    double        m_tokens    = 2048.0; ///< 当前令牌数(TokenBucket)

    // ── 漏桶状态 ──
    double        m_leakyQuota = 0.0;   ///< 当前漏桶可用配额(LeakyBucket)

    // ── 滑动窗口状态 ──
    QQueue<QPair<qint64, quint64>> m_slidingWindow; ///< 滑动窗口历史<时间戳,字节数>

    // ── 固定窗口状态 ──
    qint64        m_fixedWindowStart = 0; ///< 固定窗口起始时间戳(ms)
    quint64       m_fixedWindowUsed  = 0; ///< 固定窗口已用字节

    // ── 统计 ──
    Stats         m_stats;              ///< 统计快照
    quint64       m_statsTotalWaitTimeMs = 0; ///< 累计等待时间(用于计算均值)
    double        m_lastOutputRate   = 0.0;  ///< 上次测量的输出速率
};

#endif // DATARATELIMITER_H
