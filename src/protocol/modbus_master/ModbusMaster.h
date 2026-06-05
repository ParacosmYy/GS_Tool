/**
 * @file ModbusMaster.h
 * @brief Modbus Master轮询调度器 -- RTU帧收发、轮询定时器、重试与批量读取
 *
 * 管理Modbus RTU请求队列，支持可配置轮询间隔、超时(默认1s)、
 * 最大重试次数(默认3)、CRC16校验和批量读优化。
 * 统计接口: @see ModbusMasterStats.cpp
 */
#ifndef MODBUS_MASTER_POLL_H
#define MODBUS_MASTER_POLL_H

#include <QObject>
#include <QTimer>
#include <QQueue>
#include "protocol/modbus_master/ModbusMasterTypes.h"
#include "protocol/modbus/ModbusTypes.h"
#include "connection/interface/IConnection.h"

/**
 * @brief Modbus Master轮询调度器
 * 封装RTU帧构建/解析、轮询定时器、重试逻辑和统计追踪。
 */
class ModbusMasterPoller : public QObject {
    Q_OBJECT

public:
    /** @brief 构造轮询调度器 @param parent 父对象 */
    explicit ModbusMasterPoller(QObject* parent = nullptr);

    /** @brief 析构: 停止轮询定时器 */
    ~ModbusMasterPoller() override;

    // ---- 配置接口 ----
    /** @brief 设置串口连接 @param connection IConnection指针 */
    void setConnection(IConnection* connection);

    /** @brief 设置默认超时(毫秒) @param ms 超时值，最小10ms */
    void setTimeout(int ms);

    /** @brief 设置最大重试次数 @param count 最大重试次数 */
    void setMaxRetries(int count);

    /** @brief 设置轮询间隔(毫秒) @param ms 间隔值，最小50ms */
    void setPollInterval(int ms);

    /** @brief 获取当前超时毫秒数 @return 超时值 */
    int timeout() const;

    /** @brief 获取当前最大重试次数 @return 重试次数 */
    int maxRetries() const;

    /** @brief 获取当前轮询间隔 @return 间隔毫秒数 */
    int pollInterval() const;

    // ---- 请求接口 ----
    /** @brief 发送单次Modbus请求 @param req 请求结构体 @return true=已加入发送队列 */
    bool sendRequest(const ModbusMasterRequest& req);

    /** @brief 添加轮询条目(持续周期性请求) @param req 请求结构体 */
    void addPollEntry(const ModbusMasterRequest& req);

    /** @brief 清空所有轮询条目 */
    void clearPollEntries();

    /** @brief 启动轮询定时器 */
    void startPolling();

    /** @brief 停止轮询定时器 */
    void stopPolling();

    /** @brief 轮询是否正在运行 @return true=轮询中 */
    bool isPolling() const;

    // ---- 统计接口 ----
    quint64 totalRequests() const;    ///< @brief 累计发送请求总数
    quint64 totalResponses() const;   ///< @brief 累计成功响应总数
    quint64 totalTimeouts() const;    ///< @brief 累计超时次数
    quint64 totalErrors() const;      ///< @brief 累计错误次数
    quint64 totalRetries() const;     ///< @brief 累计重试次数
    quint64 bytesSent() const;        ///< @brief 累计发送字节数
    quint64 bytesReceived() const;    ///< @brief 累计接收字节数
    void resetStatistics();           ///< @brief 重置所有统计

signals:
    /** @brief 收到有效响应 @param response 响应结构体 */
    void responseReceived(const ModbusMasterResponse& response);

    /** @brief 请求已发送 @param request 请求结构体 */
    void requestSent(const ModbusMasterRequest& request);

    /** @brief 发生错误 @param message 错误描述 */
    void errorOccurred(const QString& message);

private slots:
    void onDataReceived(const QByteArray& data); ///< @brief 串口数据到达
    void onPollTick();                           ///< @brief 轮询定时器触发
    void onTimeout();                            ///< @brief 请求超时处理

private:
    QByteArray buildFrame(const ModbusMasterRequest& req);
    ModbusMasterResponse parseResponse(const QByteArray& raw);
    bool transmitFrame(const QByteArray& frame);
    void processNextPoll();

    IConnection*                   m_connection  = nullptr; ///< 串口连接
    QTimer*                        m_pollTimer   = nullptr; ///< 轮询定时器
    QTimer*                        m_timeoutTimer= nullptr; ///< 超时定时器
    QByteArray                     m_rxBuffer;              ///< 接收缓冲区
    QQueue<ModbusMasterRequest>    m_pollEntries;           ///< 轮询条目队列
    int                            m_pollIndex  = 0;        ///< 当前轮询索引
    ModbusMasterRequest            m_pendingReq;            ///< 当前等待响应的请求
    bool                           m_waiting    = false;    ///< 是否正在等待响应
    int                            m_retryCount = 0;        ///< 当前重试计数

    int m_timeoutMs   = 1000;  ///< 默认超时1秒
    int m_maxRetries  = 3;     ///< 默认最大重试3次
    int m_pollIntervalMs = 500; ///< 默认轮询间隔500ms

    quint64 m_totalRequests  = 0; ///< 累计发送请求
    quint64 m_totalResponses = 0; ///< 累计成功响应
    quint64 m_totalTimeouts  = 0; ///< 累计超时
    quint64 m_totalErrors    = 0; ///< 累计错误
    quint64 m_totalRetries   = 0; ///< 累计重试
    quint64 m_bytesSent      = 0; ///< 累计发送字节
    quint64 m_bytesReceived  = 0; ///< 累计接收字节
};

#endif // MODBUS_MASTER_POLL_H
