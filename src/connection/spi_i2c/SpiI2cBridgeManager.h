/** @file SpiI2cBridgeManager.h @brief SPI/I2C桥接管理器 - 运行时SPI/I2C模式切换与事务队列管理。职责: 模式切换/事务队列/桥接统计/连接生命周期。协作: SpiConnection/I2cConnection/SpiI2cConfigPanel */

#ifndef SPII2CBRIDGEMANAGER_H
#define SPII2CBRIDGEMANAGER_H

#include <QObject>
#include <QQueue>
#include <QTimer>
#include <functional>
#include "connection/spi_i2c/SpiConnection.h"
#include "connection/spi_i2c/I2cConnection.h"

/** @brief 桥接模式枚举 */
enum class BridgeMode { Spi, I2c };

/** @brief 桥接事务结构体 */
struct BridgeTransaction {
    BridgeMode mode;                           ///< 事务类型: SPI或I2C
    QByteArray txData;                         ///< 事务负载数据(发送缓冲区)
    int registerAddress = 0;                   ///< 目标寄存器地址(I2C模式)
    int deviceAddress = 0;                     ///< 目标设备地址(I2C模式)
    std::function<void(const QByteArray&)> onComplete; ///< 完成回调(参数: 接收数据)
    std::function<void(const QString&)> onError; ///< 错误回调(参数: 错误信息)
};

/** @brief SPI/I2C桥接管理器。统一管理SPI/I2C两种总线模式，共享同一串口适配器。支持运行时模式切换、事务队列、完成回调 */
class SpiI2cBridgeManager : public QObject {
    Q_OBJECT

public:
    explicit SpiI2cBridgeManager(QObject* parent = nullptr); ///< 构造
    ~SpiI2cBridgeManager();                  ///< 析构，关闭并清理资源
    void setTransport(IConnection* serial);  ///< 设置底层串口传输通道(不获取所有权)
    bool switchMode(BridgeMode mode);        ///< 切换桥接模式(自动关闭当前连接并配置新模式)
    BridgeMode currentMode() const { return m_currentMode; } ///< 当前桥接模式
    SpiConnection* spiConnection() const { return m_spiConn; } ///< SPI连接实例
    I2cConnection* i2cConnection() const { return m_i2cConn; } ///< I2C连接实例
    IConnection* activeConnection() const;   ///< 当前活跃连接(根据模式返回SPI或I2C)

    // ---- 事务队列接口 ----
    void enqueueTransaction(const BridgeTransaction& transaction); ///< 入队桥接事务
    int pendingCount() const { return m_queue.size(); } ///< 队列中待处理事务数量
    void clearQueue();                       ///< 清空事务队列
    void startQueueProcessing();             ///< 启动队列处理
    void stopQueueProcessing();              ///< 停止队列处理
    bool isProcessing() const { return m_processing; } ///< 队列是否正在处理

    // ---- 统计信息接口 ----
    quint64 totalBridgeTransactions() const { return m_totalBridgeTransactions; } ///< 总桥接事务数
    quint64 spiTransactions() const { return m_spiTransactions; }   ///< SPI事务数
    quint64 i2cTransactions() const { return m_i2cTransactions; }   ///< I2C事务数
    quint64 bridgeErrors() const { return m_bridgeErrors; }         ///< 桥接错误数
    quint64 totalModeSwitches() const { return m_totalModeSwitches; } ///< 模式切换次数
    quint64 totalQueueDrains() const { return m_totalQueueDrains; }  ///< 队列排空次数
    void resetStats();                       ///< 重置所有桥接统计计数器

signals:
    void modeSwitched(BridgeMode mode);      ///< 桥接模式切换完成信号
    void transactionCompleted(bool success); ///< 事务处理完成信号
    void queueDrained();                     ///< 队列已清空信号
    void bridgeError(const QString& errorMsg); ///< 桥接错误信号

private slots:
    void processNextTransaction();            ///< 处理队列中的下一个事务

private:
    void executeSpiTransaction(const BridgeTransaction& transaction); ///< 执行单个SPI事务
    void executeI2cTransaction(const BridgeTransaction& transaction); ///< 执行单个I2C事务

    // ---- 连接实例 ----
    SpiConnection* m_spiConn = nullptr;             ///< SPI连接实例(拥有)
    I2cConnection* m_i2cConn = nullptr;             ///< I2C连接实例(拥有)
    IConnection* m_serial = nullptr;                ///< 底层串口(不拥有)

    // ---- 模式状态 ----
    BridgeMode m_currentMode = BridgeMode::Spi;     ///< 当前桥接模式

    // ---- 事务队列 ----
    QQueue<BridgeTransaction> m_queue;              ///< 待处理事务队列
    QTimer* m_queueTimer = nullptr;                 ///< 队列处理定时器
    bool m_processing = false;                      ///< 是否正在处理

    // ---- 统计计数器 ----
    quint64 m_totalBridgeTransactions = 0;          ///< 总桥接事务数
    quint64 m_spiTransactions = 0;                  ///< SPI事务数
    quint64 m_i2cTransactions = 0;                  ///< I2C事务数
    quint64 m_bridgeErrors = 0;                     ///< 桥接错误数
    quint64 m_totalModeSwitches = 0;                ///< 模式切换次数
    quint64 m_totalQueueDrains = 0;                 ///< 队列排空次数
};

#endif // SPII2CBRIDGEMANAGER_H
