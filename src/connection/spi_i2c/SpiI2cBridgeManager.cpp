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

/** @brief 入队一个桥接事务 @param transaction 事务结构体(包含模式/数据/回调) */
void SpiI2cBridgeManager::enqueueTransaction(const BridgeTransaction& transaction)
{
    m_queue.enqueue(transaction);
    /// 如果队列处理已启动且当前未在处理，触发下一轮
    if (!m_processing && m_queueTimer && !m_queueTimer->isActive()) {
        m_queueTimer->start();
    }
}

/** @brief 清空事务队列，丢弃所有待处理事务 */
void SpiI2cBridgeManager::clearQueue()
{
    m_queue.clear();
}

/** @brief 启动队列处理(开始定时器轮询) */
void SpiI2cBridgeManager::startQueueProcessing()
{
    if (!m_processing && !m_queue.isEmpty()) {
        processNextTransaction();
    }
}

/** @brief 停止队列处理(停止定时器，当前事务不会被中断) */
void SpiI2cBridgeManager::stopQueueProcessing()
{
    if (m_queueTimer) {
        m_queueTimer->stop();
    }
    m_processing = false;
}

/** @brief 处理队列中的下一个事务，根据模式分发到SPI或I2C执行器 */
void SpiI2cBridgeManager::processNextTransaction()
{
    if (m_queue.isEmpty()) {
        m_processing = false;
        ++m_totalQueueDrains;
        emit queueDrained();
        return;
    }

    m_processing = true;
    BridgeTransaction txn = m_queue.dequeue();

    /// 根据事务模式自动切换
    if (txn.mode != m_currentMode) {
        switchMode(txn.mode);
    }

    /// 分发到对应执行器
    if (txn.mode == BridgeMode::Spi) {
        executeSpiTransaction(txn);
    } else {
        executeI2cTransaction(txn);
    }

    ++m_totalBridgeTransactions;

    /// 继续处理下一个(通过定时器异步，避免递归栈溢出)
    if (!m_queue.isEmpty() && m_queueTimer) {
        m_queueTimer->start();
    } else {
        m_processing = false;
        ++m_totalQueueDrains;
        emit queueDrained();
    }
}

/** @brief 执行单个SPI事务: 全双工传输，完成后回调 @param transaction 事务结构体 */
void SpiI2cBridgeManager::executeSpiTransaction(const BridgeTransaction& transaction)
{
    if (m_spiConn->state() != ConnectionState::Connected) {
        QString err = tr("SPI未连接，事务被丢弃");
        ++m_bridgeErrors;
        if (transaction.onError) {
            transaction.onError(err);
        }
        emit bridgeError(err);
        emit transactionCompleted(false);
        return;
    }

    QByteArray rxData = m_spiConn->transfer(transaction.txData);
    bool success = !rxData.isNull() || transaction.txData.isEmpty();

    if (success) {
        ++m_spiTransactions;
        if (transaction.onComplete) {
            transaction.onComplete(rxData);
        }
    } else {
        ++m_bridgeErrors;
        if (transaction.onError) {
            transaction.onError(tr("SPI传输失败"));
        }
    }
    emit transactionCompleted(success);
}

/** @brief 执行单个I2C事务: 寄存器读写，完成后回调 @param transaction 事务结构体 */
void SpiI2cBridgeManager::executeI2cTransaction(const BridgeTransaction& transaction)
{
    if (m_i2cConn->state() != ConnectionState::Connected) {
        QString err = tr("I2C未连接，事务被丢弃");
        ++m_bridgeErrors;
        if (transaction.onError) {
            transaction.onError(err);
        }
        emit bridgeError(err);
        emit transactionCompleted(false);
        return;
    }

    QByteArray rxData;
    bool success = false;

    if (transaction.registerAddress >= 0 && !transaction.txData.isEmpty()) {
        /// 有寄存器地址且有数据: 写寄存器
        success = m_i2cConn->writeRegister(
            transaction.deviceAddress,
            transaction.registerAddress,
            transaction.txData);
    } else if (transaction.registerAddress >= 0) {
        /// 有寄存器地址但无数据: 读寄存器
        int len = (transaction.txData.size() > 0) ? transaction.txData.size() : 1;
        rxData = m_i2cConn->readRegister(
            transaction.deviceAddress,
            transaction.registerAddress,
            len);
        success = !rxData.isNull();
    } else {
        /// 无寄存器地址: 普通写
        qint64 written = m_i2cConn->write(transaction.txData);
        success = written > 0;
    }

    if (success) {
        ++m_i2cTransactions;
        if (transaction.onComplete) {
            transaction.onComplete(rxData);
        }
    } else {
        ++m_bridgeErrors;
        if (transaction.onError) {
            transaction.onError(tr("I2C传输失败"));
        }
    }
    emit transactionCompleted(success);
}

/** @brief 重置所有桥接统计计数器 */
void SpiI2cBridgeManager::resetStats()
{
    m_totalBridgeTransactions = 0;
    m_spiTransactions = 0;
    m_i2cTransactions = 0;
    m_bridgeErrors = 0;
    m_totalModeSwitches = 0;
    m_totalQueueDrains = 0;
}
