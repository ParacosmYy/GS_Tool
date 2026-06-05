/**
 * @file ProtocolMonitor.h
 * @brief 协议监控器 -- 实时序列追踪、异常检测与时序分析
 *
 * 对协议通信进行实时监控: 按预定义的序列步骤检查消息流是否符合预期，
 * 检测异常(意外消息类型、超时、尺寸越界)，统计消息速率和异常率，
 * 并通过信号向外报告检测到的偏差。
 *
 * 协作: ProtocolEngine(协议解析)/ProtocolSession(会话管理)/ConnectionController(数据通道)
 */

#ifndef PROTOCOLMONITOR_H
#define PROTOCOLMONITOR_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>
#include <QPair>

/**
 * @brief 协议监控器
 *
 * 核心流程:
 *   1. defineSequence() 定义期望的消息序列(类型/超时/尺寸范围)
 *   2. startMonitoring() 开始监控，进入序列追踪状态
 *   3. feedMessage() 逐条喂入收到的协议消息
 *   4. 监控器自动检测: 消息类型偏差、超时、尺寸异常
 *   5. 检测到异常时发射 anomalyDetected() 信号
 *   6. 序列完成后发射 sequenceComplete()，支持自动循环
 */
class ProtocolMonitor : public QObject {
    Q_OBJECT

public:
    /** @brief 单个序列步骤的定义 */
    struct StepDef {
        QByteArray type;              ///< 期望的消息类型标识
        int timeoutMs = 5000;         ///< 本步骤允许的最大等待时间(ms)
        int minSize = 0;              ///< 消息数据最小字节数(含)
        int maxSize = 65535;          ///< 消息数据最大字节数(含)
        bool optional = false;        ///< 本步骤是否可选(可选步骤超时不计为异常)
    };

    /** @brief 异常类型枚举 */
    enum AnomalyType {
        UnexpectedMessage = 0,  ///< 意外消息类型(与期望步骤不匹配)
        Timeout = 1,            ///< 步骤等待超时
        SizeViolation = 2,      ///< 消息尺寸越界(超出[minSize, maxSize])
        SequenceDeviation = 3,  ///< 序列偏差(跳过/重复/乱序)
        ExcessiveAnomalyRate = 4 ///< 异常率超过阈值
    };
    Q_ENUM(AnomalyType)

    /** @brief 单条异常记录 */
    struct AnomalyRecord {
        AnomalyType type = UnexpectedMessage; ///< 异常类型
        QString description;                  ///< 人类可读描述
        QByteArray actualType;                ///< 实际收到的消息类型
        int expectedStepIndex = -1;           ///< 期望步骤索引(-1=无期望)
        qint64 timestampMs = 0;              ///< 异常发生时间(epoch ms)
    };

    /** @brief 监控运行统计 */
    struct MonitorStats {
        quint64 totalMessages = 0;            ///< 累计收到的消息总数
        quint64 totalAnomalies = 0;           ///< 累计异常总数
        quint64 totalTimeouts = 0;            ///< 累计超时次数
        quint64 totalSequenceDeviations = 0;  ///< 累计序列偏差次数
        quint64 totalSizeViolations = 0;      ///< 累计尺寸越界次数
        quint64 completedSequences = 0;       ///< 完整完成的序列轮次数
        double avgInterMessageDelayMs = 0.0;  ///< 平均消息间隔时间(ms)
        double messageRate = 0.0;             ///< 消息速率(条/秒)
        double anomalyRate = 0.0;             ///< 异常率(异常数/消息总数)
    };

    //-- 构造/析构 --//
    explicit ProtocolMonitor(QObject* parent = nullptr);
    ~ProtocolMonitor() override;

    //-- 序列定义 --//
    void defineSequence(const QList<StepDef>& steps);   ///< 定义期望的消息序列
    QList<StepDef> sequenceDefinition() const;          ///< 获取当前序列定义

    //-- 监控生命周期 --//
    void startMonitoring();                             ///< 开始监控(重置状态)
    void stopMonitoring();                              ///< 停止监控(停止定时器)
    bool isMonitoring() const;                          ///< 当前是否正在监控

    //-- 数据输入 --//
    void feedMessage(const QByteArray& type,
                     const QByteArray& data);           ///< 喂入一条协议消息

    //-- 配置 --//
    void setAnomalyThreshold(double rate);              ///< 设置异常率告警阈值(0.0~1.0)
    double anomalyThreshold() const;                    ///< 获取异常率告警阈值

    void setLoopEnabled(bool enabled);                  ///< 设置序列完成后是否自动循环
    bool isLoopEnabled() const;                         ///< 获取循环模式状态

    void setStrictMode(bool enabled);                   ///< 严格模式: 意外消息直接重置序列
    bool isStrictMode() const;                          ///< 获取严格模式状态

    //-- 统计 --//
    const MonitorStats& stats() const;                  ///< 获取运行统计
    void resetStatistics();                             ///< 重置所有统计计数器

    //-- 异常历史 --//
    QList<AnomalyRecord> anomalyHistory() const;        ///< 获取异常记录列表
    void clearAnomalyHistory();                         ///< 清空异常历史

    //-- 序列进度 --//
    int currentStepIndex() const;                       ///< 当前所处步骤索引(-1=未开始)
    int totalSteps() const;                             ///< 序列总步骤数

signals:
    /** @brief 检测到异常 @param type 异常类型名称 @param description 异常描述 */
    void anomalyDetected(const QString& type, const QString& description);
    /** @brief 一轮序列完整走完 */
    void sequenceComplete();
    /** @brief 序列被重置(异常/手动/循环) */
    void sequenceReset();
    /** @brief 监控状态变化 @param running true=开始 false=停止 */
    void monitoringStateChanged(bool running);

private:
    //-- 私有方法 --//
    void advanceToNextStep();                           ///< 推进到下一个序列步骤
    void resetSequencePosition();                       ///< 重置序列位置到起点
    void checkTimeoutForStep();                         ///< 检查当前步骤是否超时
    void emitAnomaly(AnomalyType type,
                     const QString& description,
                     const QByteArray& actualType);     ///< 发射异常信号并记录
    void updateStats();                                 ///< 重新计算派生统计值
    void startStepTimer();                              ///< 为当前步骤启动超时定时器

    //-- 成员变量 --//
    QList<StepDef> m_steps;                  ///< 期望的序列步骤定义
    int m_currentStep = -1;                  ///< 当前步骤索引(-1=未开始)
    bool m_monitoring = false;               ///< 是否正在监控
    bool m_loopEnabled = false;              ///< 序列完成后自动循环
    bool m_strictMode = false;               ///< 严格模式(意外消息重置)
    double m_anomalyThreshold = 0.1;         ///< 异常率告警阈值

    QTimer* m_stepTimer = nullptr;           ///< 当前步骤的超时定时器
    QElapsedTimer m_elapsed;                 ///< 全局耗时计时器
    qint64 m_lastMessageTimeMs = 0;          ///< 上一次消息的epoch时间(ms)

    //-- 统计 --//
    MonitorStats m_stats;                    ///< 运行统计
    QList<AnomalyRecord> m_anomalyHistory;   ///< 异常记录列表
    double m_sumInterMessageDelayMs = 0.0;   ///< 消息间隔总和(用于计算平均)
    quint64 m_interMessageCount = 0;         ///< 消息间隔采样数

    static constexpr int kMaxAnomalyHistory = 500; ///< 异常历史最大保留条数
};

#endif // PROTOCOLMONITOR_H
