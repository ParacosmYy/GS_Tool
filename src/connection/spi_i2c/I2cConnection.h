/**
 * @file I2cConnection.h
 * @brief I2C总线连接 - 通过I2C适配器(串口桥接)进行设备通信
 *
 * 职责:
 *   1. 提供I2C总线通信能力(设备地址/寄存器读写/突发读写)
 *   2. 支持总线扫描(7位地址0x03~0x77)和设备发现
 *   3. 通过串口桥接协议(类Bus Pirate)与I2C适配器通信
 *   4. 复用IConnection抽象接口
 *
 * 协作关系:
 *   - ConnectionFactory: 通过工厂创建实例
 *   - SpiI2cBridgeManager: 桥接管理器，协调SPI/I2C模式切换
 *   - SpiI2cConfigPanel: I2C参数配置UI
 *   - RegisterEditor: 寄存器读写编辑器
 *   - IConnection(串口): 底层传输通道
 */

#ifndef I2CCONNECTION_H
#define I2CCONNECTION_H

#include "connection/interface/IConnection.h"

/**
 * @brief I2C总线连接实现 - 通过串口桥接协议
 *
 * 使用串口作为传输通道，封装I2C-over-Serial协议。
 * 协议帧格式: [CMD(1)][LEN(2)][DATA(N)]
 * 支持标准7位地址扫描(0x03~0x77)、单字节/多字节/突发寄存器读写。
 */
class I2cConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造函数 @param parent 父对象 */
    explicit I2cConnection(QObject* parent = nullptr);

    /** @brief 析构，关闭连接 */
    ~I2cConnection() override;

    // ---- IConnection接口实现 ----
    ConnectionType type() const override;
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;
    void configure(const QVariantMap& params) override;

    // ---- I2C特有接口 ----

    /** @brief 扫描I2C总线，发现所有响应的设备(7位地址0x03~0x77) @return 发现的设备地址列表 */
    QList<int> scanBus();

    /**
     * @brief 从指定设备的寄存器读取数据(单次)
     * @param deviceAddr 设备7位地址
     * @param regAddr 寄存器地址
     * @param length 读取长度(字节)
     * @return 读取到的数据
     */
    QByteArray readRegister(int deviceAddr, int regAddr, int length);

    /**
     * @brief 向指定设备的寄存器写入数据(单次)
     * @param deviceAddr 设备7位地址
     * @param regAddr 寄存器地址
     * @param data 待写入的数据
     * @return true=写入成功
     */
    bool writeRegister(int deviceAddr, int regAddr, const QByteArray& data);

    /**
     * @brief 突发读取: 从起始寄存器连续读取多个字节
     * @param deviceAddr 设备7位地址
     * @param startReg 起始寄存器地址
     * @param count 读取字节数
     * @return 读取到的连续数据
     */
    QByteArray burstRead(int deviceAddr, int startReg, int count);

    /**
     * @brief 突发写入: 从起始寄存器连续写入多个字节
     * @param deviceAddr 设备7位地址
     * @param startReg 起始寄存器地址
     * @param data 待写入的连续数据
     * @return true=写入成功
     */
    bool burstWrite(int deviceAddr, int startReg, const QByteArray& data);

    /** @brief 设置底层串口传输通道 @param serial 串口IConnection实例(不获取所有权) */
    void setTransport(IConnection* serial);

    // ---- 统计信息接口 ----

    /** @brief 获取总传输次数 */
    quint64 totalTransfers() const { return m_totalTransactions; }

    /** @brief 获取总发送字节数 */
    quint64 totalBytesSent() const { return m_totalBytesSent; }

    /** @brief 获取总接收字节数 */
    quint64 totalBytesReceived() const { return m_totalBytesReceived; }

    /** @brief 获取错误计数 */
    quint64 errorCount() const { return m_errorCount; }

    /** @brief 获取NACK计数(设备未响应次数) */
    quint64 nackCount() const { return m_nackCount; }

    /** @brief 获取扫描发现的设备数量 */
    quint64 devicesFound() const { return m_devicesFound; }

    /** @brief 获取上次扫描发现的设备地址列表 */
    QList<int> lastScanResults() const { return m_lastScanResults; }

    /** @brief 重置所有统计计数器 */
    void resetStats();

signals:
    /** @brief 总线扫描发现设备时发出 @param address 设备7位地址 */
    void deviceFound(int address);

    /** @brief 总线扫描完成时发出 @param devices 发现的设备地址列表 */
    void scanComplete(const QList<int>& devices);

    /** @brief 寄存器读取完成时发出 @param addr 寄存器地址 @param data 读取到的数据 */
    void registerRead(int addr, const QByteArray& data);

    /** @brief 突发读取完成时发出 @param startReg 起始寄存器 @param data 读取到的数据 */
    void burstReadComplete(int startReg, const QByteArray& data);

    /** @brief NACK事件: 设备未响应 @param deviceAddr 目标设备地址 */
    void nackReceived(int deviceAddr);

private slots:
    /** @brief 底层串口数据到达回调 */
    void onTransportData(const QByteArray& data);

private:
    /** @brief 更新连接状态 */
    void updateState(ConnectionState newState);

    /** @brief 发送协议命令帧 */
    qint64 sendCommand(quint8 cmd, const QByteArray& payload);

    /** @brief 构建I2C读命令帧 */
    QByteArray buildReadFrame(int deviceAddr, int regAddr, int length);

    /** @brief 构建I2C写命令帧 */
    QByteArray buildWriteFrame(int deviceAddr, int regAddr, const QByteArray& data);

    /** @brief 构建I2C突发读命令帧 */
    QByteArray buildBurstReadFrame(int deviceAddr, int startReg, int count);

    /** @brief 构建I2C突发写命令帧 */
    QByteArray buildBurstWriteFrame(int deviceAddr, int startReg, const QByteArray& data);

    /** @brief 尝试对单个地址执行ACK探测 @param addr 7位设备地址 @return true=设备响应ACK */
    bool probeAddress(int addr);

    /** @brief 从响应缓冲区解析协议帧的负载数据 @return 负载数据，无效帧返回空 */
    QByteArray parseResponsePayload();

    // ---- 协议命令定义 ----
    static constexpr quint8 CMD_I2C_WRITE    = 0x20;  ///< I2C写命令
    static constexpr quint8 CMD_I2C_READ     = 0x21;  ///< I2C读命令
    static constexpr quint8 CMD_I2C_SCAN     = 0x22;  ///< I2C扫描命令
    static constexpr quint8 CMD_I2C_CONFIG   = 0x30;  ///< I2C配置命令
    static constexpr quint8 CMD_I2C_BURST_RD = 0x23;  ///< I2C突发读命令
    static constexpr quint8 CMD_I2C_BURST_WR = 0x24;  ///< I2C突发写命令
    static constexpr quint8 CMD_I2C_PROBE    = 0x25;  ///< I2C单地址探测

    /// 标准I2C扫描地址范围
    static constexpr int SCAN_ADDR_START = 0x03;       ///< 扫描起始地址
    static constexpr int SCAN_ADDR_END   = 0x77;       ///< 扫描结束地址

    // ---- 配置参数 ----
    int m_deviceAddress = 0x00;                     ///< 当前目标设备7位地址
    int m_clockSpeed = 100000;                       ///< I2C时钟频率(Hz)
    QString m_adapterDevice;                        ///< 适配器设备路径
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前状态

    // ---- 传输通道 ----
    IConnection* m_serial = nullptr;                ///< 底层串口连接(不拥有)
    QByteArray m_responseBuffer;                    ///< 响应数据缓冲区

    // ---- 统计计数器 ----
    quint64 m_totalTransactions = 0;                ///< 总传输次数
    quint64 m_totalBytesSent = 0;                   ///< 总发送字节数
    quint64 m_totalBytesReceived = 0;               ///< 总接收字节数
    quint64 m_errorCount = 0;                       ///< 错误计数
    quint64 m_nackCount = 0;                        ///< NACK计数(设备未响应)
    quint64 m_devicesFound = 0;                     ///< 累计发现的设备数量

    // ---- 扫描结果缓存 ----
    QList<int> m_lastScanResults;                   ///< 上次扫描发现的设备地址列表
};

#endif // I2CCONNECTION_H
