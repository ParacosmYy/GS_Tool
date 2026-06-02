/**
 * @file I2cConnection.h
 * @brief I2C总线连接 - 通过I2C适配器进行设备通信
 *
 * 职责:
 *   1. 提供I2C总线通信能力(设备地址/寄存器读写)
 *   2. 支持总线扫描和设备发现
 *   3. 复用IConnection抽象接口
 *
 * 协作关系:
 *   - ConnectionFactory: 通过工厂创建实例
 *   - SpiI2cConfigPanel: I2C参数配置UI
 *   - RegisterEditor: 寄存器读写编辑器
 */

#ifndef I2CCONNECTION_H
#define I2CCONNECTION_H

#include "connection/interface/IConnection.h"

/**
 * @brief I2C总线连接实现
 *
 * 通过USB-I2C适配器(如FT232H/CH347)进行I2C总线通信，
 * 支持设备扫描、寄存器读写等操作。
 */
class I2cConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造函数
     * @param parent 父对象
     */
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

    /**
     * @brief 扫描I2C总线，发现所有响应的设备
     * @return 发现的设备地址列表(7位地址)
     */
    QList<int> scanBus();

    /**
     * @brief 从指定设备的寄存器读取数据
     * @param deviceAddr 设备7位地址
     * @param regAddr 寄存器地址
     * @param length 读取长度(字节)
     * @return 读取到的数据
     */
    QByteArray readRegister(int deviceAddr, int regAddr, int length);

    /**
     * @brief 向指定设备的寄存器写入数据
     * @param deviceAddr 设备7位地址
     * @param regAddr 寄存器地址
     * @param data 待写入的数据
     * @return true=写入成功
     */
    bool writeRegister(int deviceAddr, int regAddr, const QByteArray& data);

signals:
    /** @brief 总线扫描发现设备时发出
     * @param address 设备7位地址
     */
    void deviceFound(int address);

    /** @brief 寄存器读取完成时发出
     * @param addr 寄存器地址
     * @param data 读取到的数据
     */
    void registerRead(int addr, const QByteArray& data);

private:
    /** @brief 更新连接状态 */
    void updateState(ConnectionState newState);

    // ---- 配置参数 ----
    int m_deviceAddress = 0x00;                     ///< 当前目标设备7位地址
    QString m_adapterDevice;                        ///< 适配器设备路径
    ConnectionState m_state = ConnectionState::Disconnected; ///< 当前状态
};

#endif // I2CCONNECTION_H
