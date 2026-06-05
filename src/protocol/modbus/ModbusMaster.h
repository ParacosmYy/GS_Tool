/**
 * @file ModbusMaster.h
 * @brief Modbus主站(客户端) -- 支持RTU/TCP双模式，8种标准功能码
 *
 * 通过IConnection(RTU串口)或QTcpSocket(TCP)发送Modbus请求帧，
 * 管理超时、CRC校验、事务ID追踪和响应解析。
 * 帧发送与响应解析: @see ModbusMasterProtocol.cpp
 * 统计接口: @see ModbusMasterStats.cpp
 */
#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

#include <QObject>
#include <QTimer>
#include <QTcpSocket>
#include <QMap>
#include <array>
#include "protocol/modbus/ModbusTypes.h"
#include "connection/interface/IConnection.h"

/** @brief Modbus主站运行时统计快照 */
struct ModbusMasterStats {
    quint64 totalRequests    = 0;  ///< 累计发送请求总数
    quint64 totalResponses   = 0;  ///< 累计接收有效响应总数
    quint64 totalTimeouts    = 0;  ///< 累计超时次数
    quint64 totalCrcErrors   = 0;  ///< 累计CRC校验失败次数
    quint64 totalExceptions  = 0;  ///< 累计Modbus异常响应次数
    quint64 bytesTransmitted = 0;  ///< 累计发送字节数
    quint64 bytesReceived    = 0;  ///< 累计接收字节数
    std::array<quint64, 8> requestsByFunctionCode = {}; ///< [0]=FC01..[7]=FC16
};

/**
 * @brief Modbus主站控制器 -- RTU/TCP双模式，构造PDU/ADU，解析响应
 */
class ModbusMaster : public QObject {
    Q_OBJECT

public:
    /** @brief Modbus传输模式 */
    enum class Mode { RTU, TCP };
    Q_ENUM(Mode)

    explicit ModbusMaster(QObject* parent = nullptr); ///< @brief 构造主站
    ~ModbusMaster() override;                         ///< @brief 析构释放TCP资源

    // ---- 配置接口 ----
    void setConnection(IConnection* connection);      ///< @brief 设置串口连接(RTU模式)
    void setSlaveAddress(int address);                ///< @brief 设置从站地址(1-247)
    void setMode(Mode mode);                          ///< @brief 设置传输模式
    void setTimeout(int ms);                          ///< @brief 设置超时(毫秒)
    void setBaudRate(int baudRate);                   ///< @brief 设置RTU波特率
    void setTcpTarget(const QString& host, quint16 port = 502); ///< @brief 设置TCP目标
    Mode mode() const;                                ///< @brief 当前传输模式
    int timeout() const;                              ///< @brief 当前超时毫秒数

    // ---- 通用请求接口 ----
    /** @brief 发送Modbus请求 @param functionCode 功能码 @param startAddress 起始地址 @param quantity 数量 @param data 写入数据(读操作忽略) @return true=已发送 */
    bool sendRequest(quint8 functionCode, quint16 startAddress,
                     quint16 quantity, const QByteArray& data = QByteArray());

    // ---- 功能码便捷方法 ----
    bool readCoils(int start, int count);              ///< @brief FC01读线圈
    bool readDiscreteInputs(int start, int count);     ///< @brief FC02读离散输入
    bool readHoldingRegisters(int start, int count);   ///< @brief FC03读保持寄存器
    bool readInputRegisters(int start, int count);     ///< @brief FC04读输入寄存器
    bool writeSingleCoil(int addr, bool on);           ///< @brief FC05写单个线圈
    bool writeSingleRegister(int addr, quint16 value); ///< @brief FC06写单个寄存器
    bool writeMultipleCoils(int start, const QList<bool>& values); ///< @brief FC15写多个线圈
    bool writeMultipleRegisters(int start, const QList<quint16>& values); ///< @brief FC16

    // ---- 统计接口 ----
    ModbusMasterStats stats() const;   ///< @brief 获取统计快照
    void resetStatistics();            ///< @brief 重置所有统计
    void resetStats();                 ///< @brief 兼容旧接口

    // ---- 兼容旧接口 ----
    bool sendCustomFrame(const ModbusFrame& frame);
    void setConnection(IConnection* connection, int slave);
    bool readRegisters(int slave, int start, int count);
    quint64 totalRequests() const;
    quint64 totalResponses() const;
    quint64 totalTimeouts() const;
    quint64 totalErrors() const;
    quint64 totalCrcErrors() const;
    quint64 successfulReads() const;
    quint64 failedReads() const;
    quint64 successfulWrites() const;
    quint64 failedWrites() const;
    quint64 totalExceptions() const;
    quint64 totalRetries() const;
    quint64 functionCodeCount(int fc) const;
    quint64 bytesTransmitted() const;
    quint64 bytesReceived() const;

signals:
    void responseReceived(const QByteArray& data, quint8 functionCode); ///< @brief 收到响应PDU
    void timeoutOccurred(quint8 functionCode);                          ///< @brief 超时
    void communicationError(const QString& message);                    ///< @brief 通信错误
    void responseReceived(const ModbusFrame& frame);  ///< @brief 兼容旧信号
    void timeout(int slave, int function);            ///< @brief 兼容旧信号
    void error(ModbusError errorCode);                ///< @brief 兼容旧信号

private slots:
    void onRawDataReceived(const QByteArray& data); ///< @brief RTU串口数据到达
    void onTcpReadyRead();                          ///< @brief TCP数据到达
    void onTimeout();                               ///< @brief 超时处理

private:
    QByteArray buildRtuAdu(quint8 slave, quint8 fc, quint16 start, quint16 qty, const QByteArray& data);
    QByteArray buildTcpAdu(quint8 fc, quint16 start, quint16 qty, const QByteArray& data);
    bool sendFrame(const QByteArray& rawData);
    void parseRtuResponse(const QByteArray& data);
    void parseTcpResponse(const QByteArray& data);
    void handleResponse(const ModbusFrame& frame);
    static int fcToIndex(quint8 fc);
    static bool isReadFunction(quint8 fc);
    static bool isWriteFunction(quint8 fc);
    void connectTcpSocket();
    void disconnectTcpSocket();

    IConnection* m_connection   = nullptr;  ///< RTU底层连接
    Mode         m_mode         = Mode::RTU;///< 传输模式
    quint8       m_slaveAddress = 1;        ///< 从站地址
    int          m_timeoutMs    = 1000;     ///< 超时毫秒
    int          m_baudRate     = 9600;     ///< RTU波特率
    QString      m_tcpHost;                 ///< TCP主机
    quint16      m_tcpPort      = 502;      ///< TCP端口
    QTcpSocket*  m_tcpSocket    = nullptr;  ///< TCP套接字
    quint16      m_transactionId= 0;        ///< TCP事务ID
    quint16      m_lastTxId     = 0;        ///< 最近事务ID
    quint8       m_lastFunction = 3;        ///< 上次功能码
    QTimer*      m_timer        = nullptr;  ///< 超时定时器
    QByteArray   m_rxBuffer;                ///< 接收缓冲区

    quint64 m_totalRequests    = 0; quint64 m_totalResponses   = 0;
    quint64 m_totalTimeouts    = 0; quint64 m_totalCrcErrors   = 0;
    quint64 m_totalErrors      = 0; quint64 m_totalExceptions  = 0;
    quint64 m_totalRetries     = 0;
    quint64 m_successfulReads  = 0; quint64 m_failedReads      = 0;
    quint64 m_successfulWrites = 0; quint64 m_failedWrites     = 0;
    quint64 m_bytesTransmitted = 0; quint64 m_bytesReceived    = 0;
    QMap<int, quint64> m_fcStats;           ///< 各功能码调用次数
};

#endif // MODBUS_MASTER_H
