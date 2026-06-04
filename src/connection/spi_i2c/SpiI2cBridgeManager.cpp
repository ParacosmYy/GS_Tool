/**
 * @file SpiI2cBridgeManager.cpp
 * @brief SPI/I2C桥接管理器实现 - 运行时模式切换与事务队列管理
 *
 * 统一管理SPI和I2C两种总线模式，共享同一个串口适配器。
 * 支持运行时模式切换(关闭当前→配置新→打开新)，
 * 以及基于定时器的事务队列串行处理。
 */

#include "connection/spi_i2c/SpiI2cBridgeManager.h"

/** @brief 构造桥接管理器，创建SPI/I2C连接实例和队列定时器 @param parent 父QObject指针 */
SpiI2cBridgeManager::SpiI2cBridgeManager(QObject* parent)
    : QObject(parent)
    , m_spiConn(new SpiConnection(this))
    , m_i2cConn(new I2cConnection(this))
    , m_queueTimer(new QTimer(this))
{
    m_queueTimer->setSingleShot(true);
    m_queueTimer->setInterval(1);  ///< 1ms间隔，尽快处理下一个事务
    connect(m_queueTimer, &QTimer::timeout,
            this, &SpiI2cBridgeManager::processNextTransaction);
}

/** @brief 析构桥接管理器，停止队列处理并断开所有连接 */
SpiI2cBridgeManager::~SpiI2cBridgeManager()
{
    stopQueueProcessing();
    if (m_spiConn) m_spiConn->close();
    if (m_i2cConn) m_i2cConn->close();
}

/** @brief 设置底层串口传输通道，同时分配给SPI和I2C连接 @param serial 串口IConnection实例(不获取所有权) */
void SpiI2cBridgeManager::setTransport(IConnection* serial)
{
    m_serial = serial;
    m_spiConn->setTransport(serial);
    m_i2cConn->setTransport(serial);
}

/** @brief 切换桥接模式(SPI↔I2C)，自动关闭当前连接并重新配置 @param mode 目标模式 @return true=切换成功 */
bool SpiI2cBridgeManager::switchMode(BridgeMode mode)
{
    if (mode == m_currentMode) {
        return true;  ///< 已经是目标模式，无需切换
    }

    /// 关闭当前模式的连接
    if (m_currentMode == BridgeMode::Spi) {
        m_spiConn->close();
    } else {
        m_i2cConn->close();
    }

    /// 设置新模式的传输通道(切换后需要重新绑定)
    if (m_serial) {
        if (mode == BridgeMode::Spi) {
            m_spiConn->setTransport(m_serial);
        } else {
            m_i2cConn->setTransport(m_serial);
        }
    }

    m_currentMode = mode;
    ++m_totalModeSwitches;
    emit modeSwitched(mode);
    return true;
}

/** @brief 获取当前活跃的连接(根据模式返回SPI或I2C) @return IConnection指针 */
IConnection* SpiI2cBridgeManager::activeConnection() const
{
    if (m_currentMode == BridgeMode::Spi) {
        return m_spiConn;
    }
    return m_i2cConn;
}

// 事务队列/执行/resetStats见 SpiI2cBridgeManagerQueue.cpp
