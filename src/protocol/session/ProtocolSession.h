/**
 * @file ProtocolSession.h
 * @brief 协议会话管理器 -- 请求/响应追踪、超时重试、P95延迟统计
 *
 * 管理完整的协议调试会话生命周期: 发送请求、自动匹配响应、超时检测、
 * 自动重试。提供响应时间的 min/max/avg/p95 统计、会话历史记录、
 * JSON导出和多维过滤功能。
 *
 * 协作: ProtocolEngine(协议解析)/ConnectionController(数据通道)/自动化测试框架
 */

#ifndef PROTOCOLSESSION_H
#define PROTOCOLSESSION_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>
#include <QJsonObject>

/**
 * @brief 协议会话管理器
 *
 * 核心流程:
 *   1. startSession() 创建新会话，进入 Idle 状态
 *   2. sendRequest() 发送请求，状态转为 WaitingResponse
 *   3. feedResponse() 喂入响应数据，按事务ID或时序自动匹配
 *   4. 超时未收到响应时自动重试(可配置次数)
 *   5. 全程记录请求/响应对，支持统计和导出
 */
class ProtocolSession : public QObject {
    Q_OBJECT

public:
    /** @brief 会话状态枚举 */
    enum State {
        Idle = 0,            ///< 空闲，等待发送请求
        WaitingResponse = 1, ///< 已发送请求，等待响应
        Timeout = 2,         ///< 响应超时
        Error = 3,           ///< 会话错误
        Complete = 4         ///< 会话完成
    };
    Q_ENUM(State)

    /** @brief 单次请求-响应对的完整记录 */
    struct TransactionRecord {
        quint64 transactionId = 0;       ///< 事务ID(自增序列号)
        QByteArray requestData;          ///< 请求数据
        QByteArray responseData;         ///< 响应数据(空=未收到响应)
        qint64 requestTimestampMs = 0;   ///< 请求发送时间(epoch ms)
        qint64 responseTimestampMs = 0;  ///< 响应接收时间(epoch ms, 0=未收到)
        double responseTimeMs = -1.0;    ///< 响应时间(ms, -1=未收到响应)
        int retryCount = 0;              ///< 该事务的重试次数
        bool matched = false;            ///< 是否已匹配到响应
        bool timedOut = false;           ///< 是否超时
        bool hadError = false;           ///< 是否发生错误
        QString errorMessage;            ///< 错误信息(空=无错误)
    };

    /** @brief 会话运行统计 */
    struct SessionStats {
        quint64 totalRequests = 0;      ///< 累计发送请求数
        quint64 totalResponses = 0;     ///< 累计收到响应数
        quint64 totalTimeouts = 0;      ///< 累计超时次数
        quint64 totalRetries = 0;       ///< 累计重试次数
        quint64 totalErrors = 0;        ///< 累计错误次数
        double avgResponseTimeMs = 0.0; ///< 平均响应时间(ms)
        double minResponseTimeMs = 0.0; ///< 最小响应时间(ms)
        double maxResponseTimeMs = 0.0; ///< 最大响应时间(ms)
        double p95ResponseTimeMs = 0.0; ///< P95响应时间(ms)
        double successRate = 0.0;       ///< 成功率(0.0~1.0)
        double timeoutRate = 0.0;       ///< 超时率(0.0~1.0)
    };

    /** @brief 过滤条件(用于查询会话历史) */
    struct FilterCriteria {
        qint64 fromTimestampMs = 0;      ///< 起始时间(0=不限制)
        qint64 toTimestampMs = 0;        ///< 截止时间(0=不限制)
        double minResponseTimeMs = -1.0; ///< 最低响应时间(-1=不限制)
        double maxResponseTimeMs = -1.0; ///< 最高响应时间(-1=不限制)
        bool includeSuccess = true;      ///< 包含成功记录
        bool includeTimeout = true;      ///< 包含超时记录
        bool includeError = true;        ///< 包含错误记录
    };

    //-- 构造/析构 --//
    explicit ProtocolSession(QObject* parent = nullptr);
    ~ProtocolSession() override;

    //-- 会话生命周期 --//
    void startSession();                                    ///< 创建新会话(重置状态和历史)
    void pauseSession();                                    ///< 暂停会话(停止超时定时器)
    void resumeSession();                                   ///< 恢复会话(重启超时定时器)
    void endSession();                                      ///< 结束会话(状态转为Complete)
    State state() const;                                    ///< 获取当前会话状态

    //-- 请求/响应 --//
    quint64 sendRequest(const QByteArray& data,
                        quint64 transactionId = 0);          ///< 发送请求(自动分配或指定事务ID)
    void feedResponse(const QByteArray& data,
                      quint64 transactionId = 0);            ///< 喂入响应(按事务ID匹配)

    //-- 配置 --//
    void setTimeoutMs(int ms);                              ///< 设置响应超时时间(ms)
    int timeoutMs() const;                                  ///< 获取当前超时时间
    void setMaxRetries(int count);                          ///< 设置最大重试次数(0=不重试)
    int maxRetries() const;                                 ///< 获取最大重试次数

    //-- 历史查询 --//
    QList<TransactionRecord> history() const;               ///< 获取完整会话历史
    QList<TransactionRecord> filteredHistory(
        const FilterCriteria& criteria) const;              ///< 按条件过滤会话历史
    void clearHistory();                                    ///< 清空会话历史

    //-- 统计 --//
    const SessionStats& stats() const;                      ///< 获取运行统计
    void resetStatistics();                                 ///< 重置所有统计计数器

    //-- 导出 --//
    QJsonObject toJson() const;                             ///< 导出完整会话为JSON对象
    bool exportToFile(const QString& filePath) const;       ///< 导出会话到JSON文件

signals:
    /** @brief 请求已发送 @param data 请求数据 */
    void requestSent(const QByteArray& data);
    /** @brief 收到响应 @param data 响应数据 */
    void responseReceived(const QByteArray& data);
    /** @brief 会话超时(最终超时，所有重试已耗尽) */
    void sessionTimeout();
    /** @brief 会话错误 @param errorMessage 错误描述 */
    void sessionError(const QString& errorMessage);
    /** @brief 会话状态变化 @param newState 新状态值 */
    void stateChanged(int newState);

private:
    //-- 私有方法 --//
    void setState(State newState);                          ///< 设置状态并发射信号
    void handleTimeout();                                   ///< 超时处理(重试或标记超时)
    void matchPendingResponse(const QByteArray& data,
                              quint64 transactionId);       ///< 匹配待处理的响应
    void updateStats();                                     ///< 重新计算全部统计值
    void recordResponseTime(double timeMs);                 ///< 记录单次响应时间到列表
    double computeP95() const;                              ///< 计算P95响应时间

    //-- 成员变量 --//
    State m_state = Idle;                    ///< 当前会话状态
    QTimer* m_timeoutTimer = nullptr;        ///< 响应超时定时器
    int m_timeoutMs = 3000;                  ///< 响应超时时间(ms)
    int m_maxRetries = 0;                    ///< 最大重试次数(0=不重试)

    quint64 m_nextTransactionId = 1;         ///< 下一个事务ID(自增)
    TransactionRecord m_pendingRequest;      ///< 当前等待响应的请求
    int m_currentRetryCount = 0;             ///< 当前请求已重试次数

    QList<TransactionRecord> m_history;      ///< 完整会话历史
    SessionStats m_stats;                    ///< 运行统计
    QList<double> m_responseTimes;           ///< 所有成功响应时间(用于P95计算)
    QElapsedTimer m_requestTimer;            ///< 请求发送计时器
};

#endif // PROTOCOLSESSION_H
