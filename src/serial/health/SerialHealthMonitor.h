/**
 * @file SerialHealthMonitor.h
 * @brief 串口健康监控器 -- 串口电气健康/信号质量/连接稳定性监控
 *
 * 专注于串口信号层面的健康监测，区别于 ConnectionHealthMonitor 的连接层监控:
 *   - 帧错误(framing error)、奇偶校验错误(parity error)、溢出错误(overrun error)、
 *     中断条件(break condition) 的逐类统计
 *   - 错误率计算: 每秒错误数、每KB错误数
 *   - 连接稳定性: 运行时间/断线时间/重连次数
 *   - 信号质量评分: 基于错误率的0-100综合评分
 *   - 突发错误检测: 在短时间窗口内聚集的连续错误簇
 *   - 错误模式分析: 区分周期性错误与随机错误(基于自相关系数)
 *   - 健康告警: 质量低于阈值时发射警告信号
 *
 * 协作关系:
 *   - SerialConnection(错误事件) → feedError() / feedHardwareError() / feedData() → SerialHealthMonitor
 *   - SerialHealthMonitor → healthScoreChanged / alertTriggered / errorBurstDetected 信号
 *   - UI层通过 getHealthReport() 获取完整健康报告
 */
#ifndef SERIALHEALTHMONITOR_H
#define SERIALHEALTHMONITOR_H
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QVariantMap>
#include <QSerialPort>
/**
 * @brief 串口健康统计数据 -- 完整的错误计数和健康指标
 */
struct SerialHealthStats {
    // ── 错误计数 ──
    quint64 totalFramingErrors = 0;       ///< 累计帧错误数(停止位不匹配)
    quint64 totalParityErrors = 0;        ///< 累计奇偶校验错误数
    quint64 totalOverrunErrors = 0;       ///< 累计溢出错误数(接收缓冲区满)
    quint64 totalBreakConditions = 0;     ///< 累计中断条件数(线路中断)
    quint64 totalErrors = 0;              ///< 累计总错误数(上述四项之和)
    quint64 totalBytesReceived = 0;       ///< 累计接收字节数
    // ── 错误率 ──
    double errorRatePerSecond = 0.0;      ///< 每秒错误数(滑动窗口)
    double errorRatePerKB = 0.0;          ///< 每KB错误数
    // ── 连接稳定性 ──
    qint64 uptimeSeconds = 0;             ///< 当前连接运行时间(秒)
    quint64 connectionDrops = 0;          ///< 累计连接断开次数
    // ── 健康评分 ──
    double healthScore = 100.0;           ///< 综合健康评分(0-100)
    // ── 突发检测 ──
    quint64 errorBursts = 0;              ///< 累计检测到的错误突发次数
};
/**
 * @brief 串口健康监控器 -- 串口信号质量/错误率/突发检测/健康评分引擎
 *
 * 使用方式:
 *   1. 构造后自动启动监控
 *   2. feedError() 喂入 Qt 串口错误事件
 *   3. feedHardwareError() 喂入底层硬件错误(帧/奇偶/溢出/中断)
 *   4. feedData() 喂入接收到的数据字节数
 *   5. stats() 随时获取统计快照
 *   6. getHealthReport() 获取完整 QVariantMap 报告
 *   7. setErrorThreshold() / setAlertEnabled() 配置告警行为
 *   8. resetStatistics() 清空所有计数器
 */
class SerialHealthMonitor : public QObject {
    Q_OBJECT
public:
    /** @brief 硬件级串口错误类型(底层驱动上报的信号错误) */
    enum class HardwareError {
        FramingError,         ///< 帧错误(停止位不匹配/波特率偏移)
        ParityError,          ///< 奇偶校验错误(校验位不匹配)
        OverrunError,         ///< 溢出错误(接收缓冲区溢出)
        BreakCondition        ///< 中断条件(线路中断/对端复位)
    };
    Q_ENUM(HardwareError)

    /** @brief 构造串口健康监控器 @param parent 父对象 */
    explicit SerialHealthMonitor(QObject* parent = nullptr);

    /** @brief 析构 -- 停止采样定时器 */
    ~SerialHealthMonitor() override;

    /**
     * @brief 喂入 Qt 串口错误事件
     *
     * 将 QSerialPort::SerialPortError 映射到统计计数器:
     *   - ResourceError → connectionDrops (串口资源丢失/设备断开)
     *   - 其他非 NoError/TimeoutError → totalOverrunErrors (通用错误桶)
     * 同时记录错误时间戳用于突发检测和模式分析。
     * @param error Qt串口错误类型
     */
    void feedError(QSerialPort::SerialPortError error);

    /**
     * @brief 喂入底层硬件错误事件
     *
     * 由上层驱动/平台代码在检测到底层信号错误时调用，
     * 精确分类到帧错误/奇偶错误/溢出错误/中断条件。
     * @param error 硬件错误类型
     */
    void feedHardwareError(HardwareError error);

    /**
     * @brief 喂入接收到的数据量
     *
     * 更新 totalBytesReceived，用于计算每KB错误率。
     * @param bytes 本次接收的字节数
     */
    void feedData(int bytes);

    /** @brief 获取当前统计数据快照 @return SerialHealthStats常量引用 */
    const SerialHealthStats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器和内部状态 */
    void resetStatistics();

    /**
     * @brief 生成完整健康报告
     *
     * 返回 QVariantMap 包含所有统计指标、健康评分、错误模式分析结果。
     * 键名: framingErrors/parityErrors/overrunErrors/breakConditions/
     *       totalErrors/totalBytes/errorRatePerSec/errorRatePerKB/
     *       uptimeSec/connectionDrops/healthScore/errorBursts/
     *       errorPattern(Periodic/Random/Unknown)
     * @return 完整健康报告
     */
    QVariantMap getHealthReport() const;

    /**
     * @brief 设置告警触发的错误率阈值
     *
     * 当错误率(每秒)超过此阈值且告警已启用时，发射 alertTriggered 信号。
     * @param errorRate 每秒错误率阈值(默认1.0)
     */
    void setErrorThreshold(double errorRate);

    /** @brief 获取当前告警阈值 @return 每秒错误率阈值 */
    double errorThreshold() const { return m_errorThreshold; }

    /**
     * @brief 启用或禁用健康告警
     * @param enabled true=启用告警(默认), false=禁用告警
     */
    void setAlertEnabled(bool enabled);

    /** @brief 查询告警是否启用 @return true=告警已启用 */
    bool isAlertEnabled() const { return m_alertEnabled; }
signals:
    /** @brief 健康评分变化 @param score 新评分(0-100) */
    void healthScoreChanged(double score);

    /** @brief 健康告警触发 @param message 告警描述文本 */
    void alertTriggered(const QString& message);

    /** @brief 检测到错误突发 @param count 突发窗口内的错误数量 */
    void errorBurstDetected(int count);
private slots:
    /** @brief 采样定时器回调 -- 每秒更新错误率和健康评分 */
    void onSampleTimer();
private:
    /** @brief 记录一次错误事件 -- 递增总错误计数和时间戳 */
    void recordError();

    /** @brief 计算综合健康评分(0-100) -- 基于错误率和连接稳定性加权 */
    double calculateHealthScore() const;

    /** @brief 更新错误率指标 -- 滑动窗口内错误/秒、错误/KB */
    void updateErrorRates();

    /** @brief 检测错误突发 -- 2秒窗口内>=3个错误视为突发 */
    void detectBurst();

    /** @brief 分析错误模式 -- 通过自相关系数区分周期性/随机错误 */
    QString analyzeErrorPattern() const;

    /** @brief 检查是否需要触发告警 */
    void checkAlert();

    // ── 定时器 ──
    QTimer m_sampleTimer;              ///< 每秒采样定时器
    QElapsedTimer m_uptimeTimer;       ///< 连接运行时间计时器

    // ── 突发检测滑动窗口 ──
    QVector<qint64> m_errorTimestamps; ///< 错误时间戳列表(ms, 用于突发检测)

    // ── 错误率滑动窗口(最近N秒) ──
    static constexpr int ERROR_WINDOW_SIZE = 10; ///< 错误率滑动窗口大小(10秒)
    int m_errorWindow[ERROR_WINDOW_SIZE] = {};   ///< 环形缓冲区: 每秒错误计数
    int m_errorWindowIdx = 0;                    ///< 环形缓冲区写入位置
    int m_currentSecondErrors = 0;               ///< 当前秒累计错误数

    // ── 配置 ──
    double m_errorThreshold = 1.0;     ///< 告警错误率阈值(错误/秒)
    bool m_alertEnabled = true;        ///< 告警开关
    double m_lastHealthScore = 100.0;  ///< 上一次健康评分(用于变化检测)
    bool m_alertActive = false;        ///< 当前是否处于告警状态(避免重复触发)

    // ── 统计 ──
    SerialHealthStats m_stats;         ///< 统计数据
};

#endif // SERIALHEALTHMONITOR_H
