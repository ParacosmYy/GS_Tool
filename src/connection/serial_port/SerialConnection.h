/**
 * @file SerialConnection.h
 * @brief 串口连接实现 — 适配器模式，封装QSerialPort到IConnection接口
 *
 * 职责: 串口参数配置(端口/波特率/数据位/校验/停止位/流控/DTR/RTS)、
 * 错误检测与分类(端口不存在/被占用/权限不足/意外断开)、信号翻译。
 */
#ifndef SERIALCONNECTION_H
#define SERIALCONNECTION_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include "connection/interface/IConnection.h"

/**
 * @brief 串口连接实现 - 封装 QSerialPort
 * 上层通过 IConnection 接口操作，无需直接接触 QSerialPort。
 * 串口错误翻译为中文描述后通过 errorOccurred 信号发出。
 */
class SerialConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造串口连接 @param parent 父对象(通常nullptr，由ConnectionManager管理生命周期) */
    explicit SerialConnection(QObject* parent = nullptr);

    /** @brief 析构函数，自动关闭串口 */
    ~SerialConnection() override;

    // ---- IConnection 接口实现 ----

    ConnectionType type() const override { return ConnectionType::Serial; } ///< 返回连接类型为 Serial
    QString name() const override;           ///< 返回端口名称(如 "COM3")
    ConnectionState state() const override;  ///< 返回当前连接状态

    /**
     * @brief 打开串口连接
     * 失败时发出详细的中文错误描述: 端口名为空/不存在/被占用/权限不足
     * @return true=打开成功, false=打开失败
     */
    bool open() override;

    /** @brief 关闭串口连接并重置状态为 Disconnected */
    void close() override;

    /** @brief 写入数据到串口 @param data 待发送的原始字节数据 @return 实际写入字节数，-1表示失败 */
    qint64 write(const QByteArray& data) override;

    /**
     * @brief 通过 QVariantMap 配置串口参数
     * 支持的 key: portName, baudRate, dataBits, parity, stopBits, flowControl, dtr, rts
     * @param params 参数映射表
     */
    void configure(const QVariantMap& params) override;

    // ---- 串口参数配置接口 ----

    void setPortName(const QString& portName);           ///< 设置端口名(如"COM3")，仅在未打开时生效
    QString portName() const;                            ///< 获取当前端口名
    void setBaudRate(qint32 baud);                       ///< 设置波特率(如115200)
    qint32 baudRate() const;                             ///< 获取当前波特率
    void setDataBits(QSerialPort::DataBits bits);        ///< 设置数据位(5/6/7/8)
    QSerialPort::DataBits dataBits() const;              ///< 获取当前数据位
    void setParity(QSerialPort::Parity parity);          ///< 设置校验模式
    QSerialPort::Parity parity() const;                  ///< 获取当前校验模式
    void setStopBits(QSerialPort::StopBits bits);        ///< 设置停止位
    QSerialPort::StopBits stopBits() const;              ///< 获取当前停止位
    void setFlowControl(QSerialPort::FlowControl control);///< 设置流控模式
    QSerialPort::FlowControl flowControl() const;        ///< 获取当前流控模式
    void setDtr(bool enabled) override;                  ///< 设置DTR信号电平
    void setRts(bool enabled) override;                  ///< 设置RTS信号电平
    bool isDtr() const override;                         ///< 查询DTR信号当前状态
    bool isRts() const override;                         ///< 查询RTS信号当前状态
    void sendBreak(int duration = 100) override;         ///< 发送Break信号
    PinoutSignals pinoutSignals() const override;        ///< 查询串口信号线电平状态

    /** @brief 获取系统中所有可用的串口列表 @return QSerialPortInfo列表 */
    static QList<QSerialPortInfo> availablePorts();

    // ---- 串口错误统计 ----

    /** @brief 获取错误计数器(只读)，覆盖IConnection默认实现 */
    SerialErrorCounters errorCounters() const override { return m_errorCounters; }

    /** @brief 重置错误计数器 */
    void resetErrorCounters();

    // ---- 错误分类统计(累计) ----

    /** @brief 获取累计已跟踪的错误总次数 @return 自上次重置以来的已跟踪错误总次数 */
    quint64 totalErrorsTracked() const { return m_totalErrorsTracked; }

    /** @brief 获取累计帧错误次数 @return 自上次重置以来的帧错误总次数 */
    quint64 totalFramingErrors() const { return m_totalFramingErrors; }

    /** @brief 获取累计校验错误次数 @return 自上次重置以来的校验错误总次数 */
    quint64 totalParityErrors() const { return m_totalParityErrors; }

    /** @brief 获取累计溢出错误次数 @return 自上次重置以来的溢出错误总次数 */
    quint64 totalOverrunErrors() const { return m_totalOverrunErrors; }

    /** @brief 重置所有错误分类统计计数器(totalErrorsTracked/totalFramingErrors/totalParityErrors/totalOverrunErrors归零) */
    void resetErrorClassificationStats();

    // ---- 操作统计 ----

    /** @brief 获取累计打开次数 @return 自上次重置以来的串口打开总次数 */
    quint64 totalOpens() const { return m_totalOpens; }

    /** @brief 获取累计关闭次数 @return 自上次重置以来的串口关闭总次数 */
    quint64 totalCloses() const { return m_totalCloses; }

    /** @brief 获取累计写入字节数 @return 自上次重置以来的串口写入字节总数 */
    quint64 totalBytesWritten() const { return m_totalBytesWritten; }

    /** @brief 获取累计读取字节数 @return 自上次重置以来的串口读取字节总数 */
    quint64 totalBytesRead() const { return m_totalBytesRead; }

    /** @brief 获取累计write()调用次数 @return 自上次重置以来的写入调用总次数 */
    quint64 totalWrites() const { return m_totalWrites; }

    /** @brief 获取累计错误次数 @return 自上次重置以来的错误总次数(含致命+可恢复) */
    quint64 errorCount() const { return m_errorCount; }

    /** @brief 重置所有操作统计计数器(totalOpens/totalCloses/totalBytesWritten/totalBytesRead/totalWrites/errorCount归零) */
    void resetStats();

private slots:
    void onReadyRead();                                  ///< QSerialPort::readyRead 信号处理
    void onError(QSerialPort::SerialPortError error);    ///< QSerialPort::errorOccurred 信号处理
    void onBytesWritten(qint64 bytes);                   ///< QSerialPort::bytesWritten 信号处理

private:
    /** @brief 将QSerialPort错误码翻译为中文错误描述 @param error 错误码 @return 中文错误描述 */
    QString translateError(QSerialPort::SerialPortError error);

    /** @brief 通过平台API获取底层串口通信错误统计
     *  Windows使用ClearCommError()读取COMSTAT错误标志(帧错误/校验错误/溢出)
     *  非Windows为空实现(未来可扩展TIOCGICOUNT) */
    void queryPlatformErrors();

    QSerialPort m_serial;                     ///< Qt 串口对象
    QString m_portName;                       ///< 端口名称，如"COM3"
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前连接状态
    SerialErrorCounters m_errorCounters;      ///< 串口错误统计计数器

    // ---- 错误分类统计计数器(累计) ----
    quint64 m_totalErrorsTracked = 0;     ///< 累计已跟踪的错误总次数
    quint64 m_totalFramingErrors = 0;     ///< 累计帧错误总次数
    quint64 m_totalParityErrors = 0;      ///< 累计校验错误总次数
    quint64 m_totalOverrunErrors = 0;     ///< 累计溢出错误总次数

    // ---- 操作统计计数器 ----
    quint64 m_totalOpens = 0;        ///< 累计串口打开次数(含成功与失败)
    quint64 m_totalCloses = 0;       ///< 累计串口关闭次数
    quint64 m_totalBytesWritten = 0; ///< 累计串口写入字节总数
    quint64 m_totalBytesRead = 0;    ///< 累计串口读取字节总数
    quint64 m_totalWrites = 0;       ///< 累计write()调用次数
    quint64 m_errorCount = 0;        ///< 累计错误总次数(致命错误+可恢复错误)
};

#endif // SERIALCONNECTION_H
