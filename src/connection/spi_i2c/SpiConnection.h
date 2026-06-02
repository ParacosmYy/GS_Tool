/**
 * @file SpiConnection.h
 * @brief SPI总线连接 - 通过SPI适配器(串口桥接)进行数据收发
 *
 * 职责:
 *   1. 提供SPI总线通信能力(模式/时钟/片选配置)
 *   2. 支持全双工transfer操作
 *   3. 通过串口桥接协议(类Aardvark)与SPI适配器通信
 *   4. 复用IConnection抽象接口
 *
 * 协作关系:
 *   - ConnectionFactory: 通过工厂创建实例
 *   - SpiI2cConfigPanel: SPI参数配置UI
 *   - IConnection(串口): 底层传输通道
 */

#ifndef SPICONNECTION_H
#define SPICONNECTION_H

#include "connection/interface/IConnection.h"

/**
 * @brief SPI总线连接实现 - 通过串口桥接协议
 *
 * 使用串口作为传输通道，封装SPI-over-Serial协议。
 * 协议帧格式: [CMD(1)][LEN(2)][DATA(N)]
 */
class SpiConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造函数
     * @param parent 父对象
     */
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

    /**
     * @brief 设置SPI模式(0~3)
     * @param mode SPI模式: CPOL/CPHA组合
     */
    void setSpiMode(int mode);

    /**
     * @brief 设置SPI时钟频率
     * @param speedHz 时钟频率(Hz)
     */
    void setClockSpeed(int speedHz);

    /**
     * @brief SPI全双工传输
     * @param txData 发送数据
     * @return 接收到的数据(MISO)
     */
    QByteArray transfer(const QByteArray& txData);

    /**
     * @brief 控制片选引脚
     * @param csPin 片选引脚编号
     * @param active true=拉低(选中)，false=拉高(释放)
     */
    void setChipSelect(int csPin, bool active);

    /**
     * @brief 设置底层串口传输通道
     * @param serial 串口IConnection实例(不获取所有权)
     */
    void setTransport(IConnection* serial);

private slots:
    /** @brief 底层串口数据到达回调 */
    void onTransportData(const QByteArray& data);

private:
    /** @brief 更新连接状态 */
    void updateState(ConnectionState newState);

    /** @brief 发送协议命令帧
     * @param cmd 命令字节
     * @param payload 负载数据
     * @return 发送字节数
     */
    qint64 sendCommand(quint8 cmd, const QByteArray& payload);

    /** @brief 组装SPI传输命令帧 */
    QByteArray buildTransferFrame(const QByteArray& txData);

    // ---- 协议命令定义 ----
    static constexpr quint8 CMD_SPI_WRITE    = 0x01;  ///< SPI写命令
    static constexpr quint8 CMD_SPI_TRANSFER = 0x02;  ///< SPI全双工传输
    static constexpr quint8 CMD_SPI_CONFIG   = 0x10;  ///< SPI配置命令
    static constexpr quint8 CMD_SPI_CS       = 0x11;  ///< SPI片选控制

    // ---- 配置参数 ----
    int m_mode = 0;                                 ///< SPI模式(0-3)
    int m_clockSpeed = 1000000;                     ///< 时钟频率(Hz)
    int m_csPin = 0;                                ///< 片选引脚编号
    QString m_adapterDevice;                        ///< 适配器设备路径
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前状态

    // ---- 传输通道 ----
    IConnection* m_serial = nullptr;                ///< 底层串口连接(不拥有)
    QByteArray m_responseBuffer;                    ///< 响应数据缓冲区
};

#endif // SPICONNECTION_H
