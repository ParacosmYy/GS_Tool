/** @file I2cConnection.h @brief I2C总线连接 - 通过串口桥接协议进行设备通信。支持7位地址扫描(0x03~0x77)/寄存器读写/突发读写 */
#ifndef I2CCONNECTION_H
#define I2CCONNECTION_H

#include "connection/interface/IConnection.h"

/**
 * @brief I2C总线连接实现 - 通过串口桥接协议
 * 协议帧格式: [CMD(1)][LEN(2)][DATA(N)]
 * 协作: ConnectionFactory(创建) / SpiI2cBridgeManager(桥接) / RegisterEditor(寄存器) / IConnection(串口通道)
 */
class I2cConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造I2C连接 @param parent 父对象 */
    explicit I2cConnection(QObject* parent = nullptr);
    /** @brief 析构，关闭连接并释放资源 */
    ~I2cConnection() override;
    // ---- IConnection接口实现 ----
    ConnectionType type() const override;              ///< 返回连接类型(I2C)
    QString name() const override;                     ///< 返回连接显示名称
    ConnectionState state() const override;            ///< 返回当前连接状态
    bool open() override;                              ///< 打开I2C连接
    void close() override;                             ///< 关闭I2C连接
    qint64 write(const QByteArray& data) override;     ///< 发送数据，返回实际写入字节数
    void configure(const QVariantMap& params) override; ///< 配置I2C参数(deviceAddress/clockSpeed等)
    // ---- I2C特有接口 ----
    QList<int> scanBus();  ///< 扫描总线(7位地址0x03~0x77)
    /** @brief 从设备寄存器读取 @param deviceAddr 7位地址 @param regAddr 寄存器地址 @param length 长度 */
    QByteArray readRegister(int deviceAddr, int regAddr, int length);
    /** @brief 向设备寄存器写入 @param deviceAddr 7位地址 @param regAddr 寄存器地址 @param data 数据 */
    bool writeRegister(int deviceAddr, int regAddr, const QByteArray& data);
    /** @brief 突发读取 @param deviceAddr 7位地址 @param startReg 起始寄存器 @param count 字节数 */
    QByteArray burstRead(int deviceAddr, int startReg, int count);
    /** @brief 突发写入 @param deviceAddr 7位地址 @param startReg 起始寄存器 @param data 数据 */
    bool burstWrite(int deviceAddr, int startReg, const QByteArray& data);
    void setTransport(IConnection* serial); ///< 设置底层串口传输通道(不获取所有权)
    // ---- 统计信息接口 ----
    quint64 totalTransactions() const { return m_totalTransactions; } ///< 总事务次数
    quint64 totalBytesWritten() const { return m_totalBytesWritten; } ///< 总写入字节数
    quint64 totalBytesRead() const { return m_totalBytesRead; }       ///< 总读取字节数
    quint64 totalNacks() const { return m_totalNacks; }               ///< 总NACK次数
    quint64 totalBusErrors() const { return m_totalBusErrors; }       ///< 总线错误次数
    quint64 totalBytesSent() const { return m_totalBytesSent; }       ///< 总发送字节数(兼容)
    quint64 totalBytesReceived() const { return m_totalBytesReceived; } ///< 总接收字节数(兼容)
    quint64 errorCount() const { return m_errorCount; }               ///< 错误计数
    quint64 nackCount() const { return m_nackCount; }                 ///< NACK计数(兼容别名)
    quint64 devicesFound() const { return m_devicesFound; }           ///< 扫描发现设备数
    QList<int> lastScanResults() const { return m_lastScanResults; }  ///< 上次扫描结果
    void resetStats();                                                ///< 重置所有统计计数器

signals:
    void deviceFound(int address);                        ///< 扫描发现设备
    void scanComplete(const QList<int>& devices);         ///< 扫描完成
    void registerRead(int addr, const QByteArray& data);  ///< 寄存器读取完成
    void burstReadComplete(int startReg, const QByteArray& data); ///< 突发读取完成
    void nackReceived(int deviceAddr);                    ///< NACK: 设备未响应

private slots:
    void onTransportData(const QByteArray& data); ///< 底层串口数据到达回调

private:
    void updateState(ConnectionState newState);  ///< 更新连接状态
    qint64 sendCommand(quint8 cmd, const QByteArray& payload); ///< 发送协议命令帧
    QByteArray buildReadFrame(int deviceAddr, int regAddr, int length);   ///< 构建I2C读命令帧
    QByteArray buildWriteFrame(int deviceAddr, int regAddr, const QByteArray& data); ///< 构建写命令帧
    QByteArray buildBurstReadFrame(int deviceAddr, int startReg, int count);  ///< 构建突发读帧
    QByteArray buildBurstWriteFrame(int deviceAddr, int startReg, const QByteArray& data); ///< 构建突发写帧
    bool probeAddress(int addr);                 ///< 单地址ACK探测
    QByteArray parseResponsePayload();           ///< 解析响应帧负载数据
    // ---- 协议命令定义 ----
    static constexpr quint8 CMD_I2C_WRITE = 0x20, CMD_I2C_READ = 0x21, CMD_I2C_SCAN = 0x22;
    static constexpr quint8 CMD_I2C_CONFIG = 0x30, CMD_I2C_BURST_RD = 0x23, CMD_I2C_BURST_WR = 0x24;
    static constexpr quint8 CMD_I2C_PROBE = 0x25;
    static constexpr int SCAN_ADDR_START = 0x03, SCAN_ADDR_END = 0x77; ///< 标准I2C扫描范围
    // ---- 配置参数 ----
    int m_deviceAddress = 0x00;                     ///< 当前目标设备7位地址
    int m_clockSpeed = 100000;                       ///< I2C时钟频率(Hz)
    QString m_adapterDevice;                        ///< 适配器设备路径
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前状态
    // ---- 传输通道 ----
    IConnection* m_serial = nullptr;                ///< 底层串口连接(不拥有)
    QByteArray m_responseBuffer;                    ///< 响应数据缓冲区
    // ---- 统计计数器 ----
    quint64 m_totalTransactions = 0, m_totalBytesSent = 0, m_totalBytesReceived = 0;
    quint64 m_totalBytesWritten = 0, m_totalBytesRead = 0;
    quint64 m_errorCount = 0, m_nackCount = 0;
    mutable quint64 m_totalNacks = 0, m_totalBusErrors = 0;
    quint64 m_devicesFound = 0;
    QList<int> m_lastScanResults;                   ///< 上次扫描结果缓存
};

#endif // I2CCONNECTION_H
