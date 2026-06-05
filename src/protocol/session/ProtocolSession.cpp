/**
 * @file ProtocolSession.cpp
 * @brief 协议会话管理器实现 -- 生命周期/请求响应匹配/超时重试/统计/导出
 *
 * 核心流程:
 *   1. startSession() 重置会话状态,进入Idle
 *   2. sendRequest() 记录请求,启动超时定时器,状态转为WaitingResponse
 *   3. feedResponse() 按事务ID匹配或按时序匹配,记录响应时间
 *   4. handleTimeout() 超时后根据重试配置决定重发或标记超时
 *   5. updateStats() 全量重算统计值(含P95)
 *
 * 统计重置方法见 ProtocolSessionStats.cpp。
 */

#include "protocol/session/ProtocolSession.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDateTime>

// ============================================================================
// 构造 / 析构
// ============================================================================

/** @brief 构造协议会话管理器 @param parent 父对象 */
ProtocolSession::ProtocolSession(QObject* parent)
    : QObject(parent)
    , m_timeoutTimer(new QTimer(this))
{
    setObjectName("ProtocolSession");
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout,
            this, &ProtocolSession::handleTimeout);
}

/** @brief 析构(停止定时器) */
ProtocolSession::~ProtocolSession()
{
    if (m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }
}

// ============================================================================
// 会话生命周期
// ============================================================================

/**
 * @brief 创建新会话
 *
 * 重置所有状态、历史和统计，进入Idle状态。
 * 调用后可以开始新的请求-响应调试序列。
 */
void ProtocolSession::startSession()
{
    m_timeoutTimer->stop();
    m_history.clear();
    m_responseTimes.clear();
    m_pendingRequest = TransactionRecord();
    m_currentRetryCount = 0;
    m_nextTransactionId = 1;
    resetStatistics();
    setState(Idle);
}

/**
 * @brief 暂停会话
 *
 * 停止超时定时器，保持当前状态和历史不变。
 * 暂停期间feedResponse()仍可正常匹配(但不会触发新的超时)。
 */
void ProtocolSession::pauseSession()
{
    m_timeoutTimer->stop();
}

/**
 * @brief 恢复会话
 *
 * 如果当前有等待中的请求，重新启动超时定时器。
 */
void ProtocolSession::resumeSession()
{
    if (m_state == WaitingResponse && !m_pendingRequest.requestData.isEmpty()) {
        m_timeoutTimer->start(m_timeoutMs);
    }
}

/**
 * @brief 结束会话
 *
 * 停止超时定时器，将状态设为Complete。
 * 保留历史记录供后续查询和导出。
 */
void ProtocolSession::endSession()
{
    m_timeoutTimer->stop();
    setState(Complete);
}

/** @brief 获取当前会话状态 @return State枚举值 */
ProtocolSession::State ProtocolSession::state() const
{
    return m_state;
}

// ============================================================================
// 请求 / 响应
// ============================================================================

/**
 * @brief 发送请求
 * @param data 请求数据
 * @param transactionId 事务ID(0=自动分配)
 * @return 分配的事务ID
 *
 * 流程:
 *   1. 创建事务记录,记录请求数据和发送时间
 *   2. 启动超时定时器
 *   3. 状态转为WaitingResponse
 *   4. 发射requestSent信号
 */
quint64 ProtocolSession::sendRequest(const QByteArray& data, quint64 transactionId)
{
    if (data.isEmpty()) {
        return 0;
    }

    /* 分配事务ID */
    quint64 tid = (transactionId > 0) ? transactionId : m_nextTransactionId++;
    if (tid >= m_nextTransactionId) {
        m_nextTransactionId = tid + 1;
    }

    /* 创建事务记录 */
    m_pendingRequest = TransactionRecord();
    m_pendingRequest.transactionId = tid;
    m_pendingRequest.requestData = data;
    m_pendingRequest.requestTimestampMs = QDateTime::currentMSecsSinceEpoch();
    m_pendingRequest.retryCount = m_currentRetryCount;
    m_state = WaitingResponse;

    /* 启动计时器和超时定时器 */
    m_requestTimer.start();
    m_timeoutTimer->start(m_timeoutMs);

    m_stats.totalRequests++;
    emit requestSent(data);
    return tid;
}

/**
 * @brief 喂入响应数据
 * @param data 响应数据
 * @param transactionId 关联的事务ID(0=自动匹配最早未完成的请求)
 *
 * 匹配逻辑:
 *   1. 指定transactionId时精确匹配
 *   2. transactionId为0时匹配当前等待中的请求
 *   3. 匹配成功后计算响应时间，更新统计
 */
void ProtocolSession::feedResponse(const QByteArray& data, quint64 transactionId)
{
    if (data.isEmpty()) {
        return;
    }

    /* 尝试匹配 */
    matchPendingResponse(data, transactionId);
}

// ============================================================================
// 配置
// ============================================================================

/** @brief 设置响应超时时间 @param ms 超时毫秒数(必须>0) */
void ProtocolSession::setTimeoutMs(int ms)
{
    m_timeoutMs = (ms > 0) ? ms : 3000;
}

/** @brief 获取当前超时时间 @return 超时毫秒数 */
int ProtocolSession::timeoutMs() const
{
    return m_timeoutMs;
}

/** @brief 设置最大重试次数 @param count 最大次数(0=不重试) */
void ProtocolSession::setMaxRetries(int count)
{
    m_maxRetries = (count >= 0) ? count : 0;
}

/** @brief 获取最大重试次数 @return 最大次数 */
int ProtocolSession::maxRetries() const
{
    return m_maxRetries;
}

// ============================================================================
// 历史查询
// ============================================================================

/** @brief 获取完整会话历史 @return 事务记录列表副本 */
QList<ProtocolSession::TransactionRecord> ProtocolSession::history() const
{
    return m_history;
}

/**
 * @brief 按条件过滤会话历史
 * @param criteria 过滤条件
 * @return 符合条件的事务记录列表
 *
 * 过滤维度: 时间范围、响应时间范围、成功/超时/错误类型。
 * 所有条件取交集(AND关系)。
 */
QList<ProtocolSession::TransactionRecord> ProtocolSession::filteredHistory(
    const FilterCriteria& criteria) const
{
    QList<TransactionRecord> result;
    for (const TransactionRecord& rec : m_history) {
        /* 时间范围过滤 */
        if (criteria.fromTimestampMs > 0 &&
            rec.requestTimestampMs < criteria.fromTimestampMs) {
            continue;
        }
        if (criteria.toTimestampMs > 0 &&
            rec.requestTimestampMs > criteria.toTimestampMs) {
            continue;
        }

        /* 响应时间范围过滤(仅对已匹配的记录) */
        if (rec.matched && rec.responseTimeMs >= 0) {
            if (criteria.minResponseTimeMs >= 0 &&
                rec.responseTimeMs < criteria.minResponseTimeMs) {
                continue;
            }
            if (criteria.maxResponseTimeMs >= 0 &&
                rec.responseTimeMs > criteria.maxResponseTimeMs) {
                continue;
            }
        }

        /* 状态类型过滤 */
        if (!criteria.includeSuccess && rec.matched && !rec.timedOut && !rec.hadError) {
            continue;
        }
        if (!criteria.includeTimeout && rec.timedOut) {
            continue;
        }
        if (!criteria.includeError && rec.hadError) {
            continue;
        }

        result.append(rec);
    }
    return result;
}

/** @brief 清空会话历史(不影响当前状态) */
void ProtocolSession::clearHistory()
{
    m_history.clear();
    m_responseTimes.clear();
}

// ============================================================================
// 统计
// ============================================================================

/** @brief 获取运行统计 @return SessionStats的const引用 */
const ProtocolSession::SessionStats& ProtocolSession::stats() const
{
    return m_stats;
}

// ============================================================================
// 导出
// ============================================================================

/**
 * @brief 导出完整会话为JSON对象
 * @return QJsonObject 包含会话元数据和完整历史记录
 *
 * JSON结构:
 * {
 *   "session": { "startTime", "totalTransactions", "state" },
 *   "config": { "timeoutMs", "maxRetries" },
 *   "stats": { ... },
 *   "history": [ { "transactionId", "request", "response", ... } ]
 * }
 */
QJsonObject ProtocolSession::toJson() const
{
    QJsonObject root;

    /* 会话元数据 */
    QJsonObject sessionObj;
    sessionObj["totalTransactions"] = static_cast<qint64>(m_history.size());
    sessionObj["state"] = static_cast<int>(m_state);
    root["session"] = sessionObj;

    /* 配置 */
    QJsonObject configObj;
    configObj["timeoutMs"] = m_timeoutMs;
    configObj["maxRetries"] = m_maxRetries;
    root["config"] = configObj;

    /* 统计 */
    QJsonObject statsObj;
    statsObj["totalRequests"] = static_cast<qint64>(m_stats.totalRequests);
    statsObj["totalResponses"] = static_cast<qint64>(m_stats.totalResponses);
    statsObj["totalTimeouts"] = static_cast<qint64>(m_stats.totalTimeouts);
    statsObj["totalRetries"] = static_cast<qint64>(m_stats.totalRetries);
    statsObj["totalErrors"] = static_cast<qint64>(m_stats.totalErrors);
    statsObj["avgResponseTimeMs"] = m_stats.avgResponseTimeMs;
    statsObj["minResponseTimeMs"] = m_stats.minResponseTimeMs;
    statsObj["maxResponseTimeMs"] = m_stats.maxResponseTimeMs;
    statsObj["p95ResponseTimeMs"] = m_stats.p95ResponseTimeMs;
    statsObj["successRate"] = m_stats.successRate;
    statsObj["timeoutRate"] = m_stats.timeoutRate;
    root["stats"] = statsObj;

    /* 历史记录 */
    QJsonArray historyArr;
    for (const TransactionRecord& rec : m_history) {
        QJsonObject recObj;
        recObj["transactionId"] = static_cast<qint64>(rec.transactionId);
        recObj["requestData"] = QString::fromUtf8(rec.requestData.toHex());
        recObj["responseData"] = QString::fromUtf8(rec.responseData.toHex());
        recObj["requestTimestampMs"] = rec.requestTimestampMs;
        recObj["responseTimestampMs"] = rec.responseTimestampMs;
        recObj["responseTimeMs"] = rec.responseTimeMs;
        recObj["retryCount"] = rec.retryCount;
        recObj["matched"] = rec.matched;
        recObj["timedOut"] = rec.timedOut;
        recObj["hadError"] = rec.hadError;
        if (!rec.errorMessage.isEmpty()) {
            recObj["errorMessage"] = rec.errorMessage;
        }
        historyArr.append(recObj);
    }
    root["history"] = historyArr;

    return root;
}

/**
 * @brief 导出会话到JSON文件
 * @param filePath 目标文件路径
 * @return true=导出成功 false=写入失败
 */
bool ProtocolSession::exportToFile(const QString& filePath) const
{
    QJsonDocument doc(toJson());
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

// ============================================================================
// 私有方法
// ============================================================================

/**
 * @brief 设置状态并发射信号
 * @param newState 新状态
 */
void ProtocolSession::setState(State newState)
{
    if (m_state == newState) {
        return;
    }
    m_state = newState;
    emit stateChanged(static_cast<int>(newState));
}

/**
 * @brief 超时处理
 *
 * 超时后的处理逻辑:
 *   1. 如果未超过最大重试次数，重发请求并递增重试计数
 *   2. 超过最大重试次数，标记该事务为超时，记录到历史
 *   3. 更新统计，发射sessionTimeout信号
 */
void ProtocolSession::handleTimeout()
{
    if (m_pendingRequest.requestData.isEmpty()) {
        return;
    }

    m_currentRetryCount++;
    m_stats.totalTimeouts++;

    /* 尝试重试 */
    if (m_currentRetryCount <= m_maxRetries) {
        m_stats.totalRetries++;
        m_pendingRequest.retryCount = m_currentRetryCount;
        m_requestTimer.start();
        m_timeoutTimer->start(m_timeoutMs);
        emit requestSent(m_pendingRequest.requestData);
        return;
    }

    /* 重试耗尽，标记超时 */
    TransactionRecord record = m_pendingRequest;
    record.timedOut = true;
    record.matched = false;
    m_history.append(record);

    m_pendingRequest = TransactionRecord();
    m_currentRetryCount = 0;
    setState(Timeout);
    updateStats();
    emit sessionTimeout();
}

/**
 * @brief 匹配待处理的响应
 * @param data 响应数据
 * @param transactionId 事务ID(0=自动匹配)
 *
 * 匹配成功后:
 *   1. 计算响应时间(elapsed timer)
 *   2. 完善事务记录并存入历史
 *   3. 重置待处理请求，更新统计
 */
void ProtocolSession::matchPendingResponse(const QByteArray& data, quint64 transactionId)
{
    /* 无待处理请求时，无法匹配 */
    if (m_pendingRequest.requestData.isEmpty()) {
        return;
    }

    /* 按事务ID匹配: 指定ID时必须精确匹配 */
    if (transactionId > 0 && transactionId != m_pendingRequest.transactionId) {
        return;
    }

    /* 匹配成功 */
    m_timeoutTimer->stop();

    double elapsed = static_cast<double>(m_requestTimer.elapsed());

    TransactionRecord record = m_pendingRequest;
    record.responseData = data;
    record.responseTimestampMs = QDateTime::currentMSecsSinceEpoch();
    record.responseTimeMs = elapsed;
    record.matched = true;
    m_history.append(record);

    /* 记录响应时间用于统计 */
    recordResponseTime(elapsed);

    m_stats.totalResponses++;

    /* 重置待处理状态 */
    m_pendingRequest = TransactionRecord();
    m_currentRetryCount = 0;
    setState(Idle);
    updateStats();
    emit responseReceived(data);
}
