/** @file SpiConnection.h @brief SPI总线连接 - 通过SPI适配器(串口桥接)进行数据收发。支持CPOL/CPHA模式0-3/时钟/位序/字长/CS极性/全双工transfer。协作: ConnectionFactory/SpiI2cBridgeManager/SpiI2cConfigPanel */

#ifndef SPICONNECTION_H
#define SPICONNECTION_H

#include "connection/interface/IConnection.h"

/** @brief SPI位序枚举 */
enum class SpiBitOrder { MSB, LSB };

/** @brief SPI字长枚举 */
enum class SpiWordSize { Bit8 = 8, Bit16 = 16, Bit32 = 32 };

/** @brief SPI总线连接实现。使用串口桥接协议，帧格式: [CMD(1)][LEN(2)][DATA(N)]。支持模式0-3/字长8-16-32/MSB-LSB/CS极性 */
class SpiConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造SPI连接 @param parent 父对象 */
    explicit SpiConnection(QObject* parent = nullptr);
    /** @brief 析构，关闭连接并释放资源 */
    ~SpiConnection() override;
    // ---- IConnection接口 ----
    ConnectionType type() const override;              ///< 返回连接类型(SPI)
    QString name() const override;                     ///< 返回连接显示名称
    ConnectionState state() const override;            ///< 返回当前连接状态
    bool open() override;                              ///< 打开SPI连接
    void close() override;                             ///< 关闭SPI连接
    qint64 write(const QByteArray& data) override;     ///< 发送数据，返回实际写入字节数
    void configure(const QVariantMap& params) override; ///< 配置SPI参数(mode/clockSpeed/device等)
    // ---- SPI特有接口 ----
    /** @brief 设置SPI模式(0~3) @param mode SPI模式编号 */
    void setSpiMode(int mode);
    /** @brief 设置时钟频率 @param speedHz 时钟频率(Hz) */
    void setClockSpeed(int speedHz);
    /** @brief 设置位序 @param order MSB或LSB */
    void setBitOrder(SpiBitOrder order);
    /** @brief 设置字长 @param wordSize 8/16/32位 */
    void setWordSize(SpiWordSize wordSize);
    /** @brief 设置CS极性 @param activeLow true=低有效 */
    void setCsPolarity(bool activeLow);
    /** @brief SPI全双工传输 @param txData 发送数据 @return 接收到的数据 */
    QByteArray transfer(const QByteArray& txData);
    /** @brief SPI全双工传输(指定字长) @param txData 发送数据 @param wordSize 字长 @return 接收到的数据 */
    QByteArray transfer(const QByteArray& txData, SpiWordSize wordSize);
    /** @brief 控制片选引脚 @param csPin 片选引脚编号 @param active true=激活 */
    void setChipSelect(int csPin, bool active);
    /** @brief 设置底层串口传输通道(不获取所有权) @param serial 底层串口连接 */
    void setTransport(IConnection* serial);

    // ---- 统计信息接口 ----
    quint64 totalTransfers() const { return m_totalTransfers; }  ///< 总传输次数
    quint64 totalBytesSent() const { return m_totalBytesSent; }   ///< 总发送字节数
    quint64 totalBytesReceived() const { return m_totalBytesReceived; } ///< 总接收字节数
    quint64 totalTransferErrors() const { return m_totalTransferErrors; } ///< 总传输错误次数
    quint64 totalCsToggles() const { return m_totalCsToggles; }   ///< 总CS片选切换次数
    quint64 errorCount() const { return m_errorCount; }           ///< 错误计数
    quint64 transferByMode(int mode) const;  ///< 指定SPI模式(0-3)的传输次数
    int spiMode() const { return m_mode; }   ///< 当前SPI模式
    int clockSpeed() const { return m_clockSpeed; } ///< 当前时钟频率
    SpiBitOrder bitOrder() const { return m_bitOrder; } ///< 当前位序
    SpiWordSize wordSize() const { return m_wordSize; } ///< 当前字长
    bool csActiveLow() const { return m_csActiveLow; } ///< 当前CS极性
    void resetStats();                       ///< 重置所有统计计数器

signals:
    void transferCompleted(int txBytes, int rxBytes); ///< SPI传输完成信号

private slots:
    void onTransportData(const QByteArray& data); ///< 底层串口数据到达回调

private:
    void updateState(ConnectionState newState); ///< 更新连接状态
    qint64 sendCommand(quint8 cmd, const QByteArray& payload); ///< 发送协议命令帧
    QByteArray buildTransferFrame(const QByteArray& txData); ///< 组装SPI传输命令帧
    QByteArray buildConfigFrame();            ///< 组装SPI配置命令帧(含位序/字长/CS极性)

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
    mutable quint64 m_totalTransferErrors = 0;      ///< 总传输错误次数
    mutable quint64 m_totalCsToggles = 0;           ///< 总CS片选切换次数
    quint64 m_transferByMode[4] = {0, 0, 0, 0};    ///< 按SPI模式统计传输次数
};

#endif // SPICONNECTION_H
