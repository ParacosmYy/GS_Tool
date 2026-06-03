/**
 * @file SpiConnection.h
 * @brief SPI总线连接 - 通过SPI适配器(串口桥接)进行数据收发
 *
 * 职责:
 *   1. 提供SPI总线通信能力(CPOL/CPHA模式0-3/时钟/位序/字长/CS极性)
 *   2. 支持全双工transfer操作(8/16/32位字长)
 *   3. 通过串口桥接协议(类Aardvark)与SPI适配器通信
 *   4. 复用IConnection抽象接口
 *
 * 协作关系:
 *   - ConnectionFactory: 通过工厂创建实例
 *   - SpiI2cBridgeManager: 桥接管理器，协调SPI/I2C模式切换
 *   - SpiI2cConfigPanel: SPI参数配置UI
 *   - IConnection(串口): 底层传输通道
 */

#ifndef SPICONNECTION_H
#define SPICONNECTION_H

#include "connection/interface/IConnection.h"

/**
 * @brief SPI位序枚举
 */
enum class SpiBitOrder {
    MSB,    ///< 高位在前(默认)
    LSB     ///< 低位在前
};

/**
 * @brief SPI字长枚举
 */
enum class SpiWordSize {
    Bit8  = 8,   ///< 8位字长(默认)
    Bit16 = 16,  ///< 16位字长
    Bit32 = 32   ///< 32位字长
};

/**
 * @brief SPI总线连接实现 - 通过串口桥接协议
 *
 * 使用串口作为传输通道，封装SPI-over-Serial协议。
 * 协议帧格式: [CMD(1)][LEN(2)][DATA(N)]
 * 支持SPI模式0-3(CPOL/CPHA组合)、可配置字长(8/16/32位)、
 * MSB/LSB位序、CS极性(低有效/高有效)。
 */
class SpiConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造函数 @param parent 父对象 */
    explicit SpiConnection(QObject* parent = nullptr);

    /** @brief 析构，关闭连接 */
    ~SpiConnection() override;

    // ---- IConnection接口实现 ----
    ConnectionType type() const override;
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;
    void configure(const QVariantMap& params) override;

    // ---- SPI特有接口 ----

    /** @brief 设置SPI模式(0~3) @param mode CPOL/CPHA组合模式 */
    void setSpiMode(int mode);

    /** @brief 设置SPI时钟频率 @param speedHz 时钟频率(Hz) */
    void setClockSpeed(int speedHz);

    /** @brief 设置位序 @param order MSB/LSB位序 */
    void setBitOrder(SpiBitOrder order);

    /** @brief 设置字长 @param wordSize 8/16/32位 */
    void setWordSize(SpiWordSize wordSize);

    /** @brief 设置CS极性 @param activeLow true=低电平有效(默认)，false=高电平有效 */
    void setCsPolarity(bool activeLow);

    /** @brief SPI全双工传输 @param txData 发送数据 @return 接收到的数据(MISO) */
    QByteArray transfer(const QByteArray& txData);

    /**
     * @brief SPI全双工传输(指定字长)
     * @param txData 发送数据
     * @param wordSize 本次传输使用的字长
     * @return 接收到的数据(MISO)
     */
    QByteArray transfer(const QByteArray& txData, SpiWordSize wordSize);

    /** @brief 控制片选引脚 @param csPin 片选引脚编号 @param active true=选中，false=释放 */
    void setChipSelect(int csPin, bool active);

    /** @brief 设置底层串口传输通道 @param serial 串口IConnection实例(不获取所有权) */
    void setTransport(IConnection* serial);

    // ---- 统计信息接口 ----

    /** @brief 获取总传输次数 */
    quint64 totalTransfers() const { return m_totalTransfers; }

    /** @brief 获取总发送字节数 */
    quint64 totalBytesSent() const { return m_totalBytesSent; }

    /** @brief 获取总接收字节数 */
    quint64 totalBytesReceived() const { return m_totalBytesReceived; }

    /** @brief 获取错误计数 */
    quint64 errorCount() const { return m_errorCount; }

    /** @brief 获取指定SPI模式的传输次数 @param mode SPI模式(0-3) @return 该模式累计传输次数 */
    quint64 transferByMode(int mode) const;

    /** @brief 获取当前配置的SPI模式 */
    int spiMode() const { return m_mode; }

    /** @brief 获取当前配置的时钟频率 */
    int clockSpeed() const { return m_clockSpeed; }

    /** @brief 获取当前配置的位序 */
    SpiBitOrder bitOrder() const { return m_bitOrder; }

    /** @brief 获取当前配置的字长 */
    SpiWordSize wordSize() const { return m_wordSize; }

    /** @brief 获取当前CS极性(低电平有效) */
    bool csActiveLow() const { return m_csActiveLow; }

    /** @brief 重置所有统计计数器 */
    void resetStats();

signals:
    /** @brief SPI传输完成信号 @param txBytes 发送字节数 @param rxBytes 接收字节数 */
    void transferCompleted(int txBytes, int rxBytes);

private slots:
    /** @brief 底层串口数据到达回调 */
    void onTransportData(const QByteArray& data);

private:
    /** @brief 更新连接状态 */
    void updateState(ConnectionState newState);

    /** @brief 发送协议命令帧 @param cmd 命令字节 @param payload 负载数据 @return 发送字节数 */
    qint64 sendCommand(quint8 cmd, const QByteArray& payload);

    /** @brief 组装SPI传输命令帧 */
    QByteArray buildTransferFrame(const QByteArray& txData);

    /** @brief 组装SPI配置命令帧(含位序/字长/CS极性) */
    QByteArray buildConfigFrame();

    // ---- 协议命令定义 ----
    static constexpr quint8 CMD_SPI_WRITE    = 0x01;  ///< SPI写命令
    static constexpr quint8 CMD_SPI_TRANSFER = 0x02;  ///< SPI全双工传输
    static constexpr quint8 CMD_SPI_CONFIG   = 0x10;  ///< SPI配置命令
    static constexpr quint8 CMD_SPI_CS       = 0x11;  ///< SPI片选控制

    // ---- 配置参数 ----
    int m_mode = 0;                                 ///< SPI模式(0-3)
    int m_clockSpeed = 1000000;                     ///< 时钟频率(Hz)
    int m_csPin = 0;                                ///< 片选引脚编号
    SpiBitOrder m_bitOrder = SpiBitOrder::MSB;      ///< 位序
    SpiWordSize m_wordSize = SpiWordSize::Bit8;     ///< 字长
    bool m_csActiveLow = true;                      ///< CS低电平有效
    QString m_adapterDevice;                        ///< 适配器设备路径
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前状态

    // ---- 传输通道 ----
    IConnection* m_serial = nullptr;                ///< 底层串口连接(不拥有)
    QByteArray m_responseBuffer;                    ///< 响应数据缓冲区

    // ---- 统计计数器 ----
    quint64 m_totalTransfers = 0;                   ///< 总传输次数
    quint64 m_totalBytesSent = 0;                   ///< 总发送字节数
    quint64 m_totalBytesReceived = 0;               ///< 总接收字节数
    quint64 m_errorCount = 0;                       ///< 错误计数
    quint64 m_transferByMode[4] = {0, 0, 0, 0};    ///< 按SPI模式统计传输次数
};

#endif // SPICONNECTION_H
