/**
 * @file SpiConnection.h
 * @brief SPI总线连接 - 通过SPI适配器进行数据收发
 *
 * 职责:
 *   1. 提供SPI总线通信能力(模式/时钟/片选配置)
 *   2. 支持全双工transfer操作
 *   3. 复用IConnection抽象接口
 *
 * 协作关系:
 *   - ConnectionFactory: 通过工厂创建实例
 *   - SpiI2cConfigPanel: SPI参数配置UI
 */

#ifndef SPICONNECTION_H
#define SPICONNECTION_H

#include "connection/interface/IConnection.h"

/**
 * @brief SPI总线连接实现
 *
 * 通过USB-SPI适配器(如FT232H/CH347)进行SPI总线通信，
 * 支持配置SPI模式(0-3)、时钟频率和片选引脚。
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

private:
    /** @brief 更新连接状态 */
    void updateState(ConnectionState newState);

    // ---- 配置参数 ----
    int m_mode = 0;                                 ///< SPI模式(0-3)
    int m_clockSpeed = 1000000;                     ///< 时钟频率(Hz)
    int m_csPin = 0;                                ///< 片选引脚编号
    QString m_adapterDevice;                        ///< 适配器设备路径
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前状态
};

#endif // SPICONNECTION_H
