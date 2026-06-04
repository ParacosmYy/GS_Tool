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
    /** @brief 构造SPI/I2C桥接管理器 @param parent 父对象 */
    explicit SpiI2cBridgeManager(QObject* parent = nullptr);
    /** @brief 析构，关闭并清理资源 */
    ~SpiI2cBridgeManager();
    /** @brief 设置底层串口传输通道(不获取所有权) @param serial 底层串口连接 */
    void setTransport(IConnection* serial);
    /** @brief 切换桥接模式(自动关闭当前连接并配置新模式) @param mode 目标模式 @return true=切换成功 */
    bool switchMode(BridgeMode mode);
    BridgeMode currentMode() const { return m_currentMode; } ///< @return 当前桥接模式
    SpiConnection* spiConnection() const { return m_spiConn; } ///< @return SPI连接实例
    I2cConnection* i2cConnection() const { return m_i2cConn; } ///< @return I2C连接实例
    /** @brief 获取当前活跃连接(根据模式返回SPI或I2C) @return 活跃连接指针 */
    IConnection* activeConnection() const;

    // ---- 事务队列接口 ----
    /** @brief 入队桥接事务 @param transaction 桥接事务 */
    void enqueueTransaction(const BridgeTransaction& transaction);
    /** @brief 获取队列中待处理事务数量 @return 待处理事务数 */
    int pendingCount() const { return m_queue.size(); }
    /** @brief 清空事务队列 */
    void clearQueue();
    /** @brief 启动队列处理 */
    void startQueueProcessing();
    /** @brief 停止队列处理 */
    void stopQueueProcessing();
    /** @brief 查询队列是否正在处理 @return true=正在处理 */
    bool isProcessing() const { return m_processing; }

    // ---- 统计信息接口 ----
    /** @brief 获取总桥接事务数 @return 累计事务处理次数 */
    quint64 totalBridgeTransactions() const { return m_totalBridgeTransactions; }
    /** @brief 获取SPI事务数 @return 累计SPI事务次数 */
    quint64 spiTransactions() const { return m_spiTransactions; }
    /** @brief 获取I2C事务数 @return 累计I2C事务次数 */
    quint64 i2cTransactions() const { return m_i2cTransactions; }
    /** @brief 获取桥接错误数 @return 累计错误次数 */
    quint64 bridgeErrors() const { return m_bridgeErrors; }
    /** @brief 获取模式切换次数 @return 累计模式切换次数 */
    quint64 totalModeSwitches() const { return m_totalModeSwitches; }
    /** @brief 获取队列排空次数 @return 累计队列排空次数 */
    quint64 totalQueueDrains() const { return m_totalQueueDrains; }
    /** @brief 重置所有桥接统计计数器 */
    void resetStats();

signals:
    /** @brief 桥接模式切换完成信号 @param mode 切换后的模式 */
    void modeSwitched(BridgeMode mode);
    /** @brief 事务处理完成信号 @param success true=成功 */
    void transactionCompleted(bool success);
    /** @brief 队列已清空信号 */
    void queueDrained();
    /** @brief 桥接错误信号 @param errorMsg 错误信息 */
    void bridgeError(const QString& errorMsg);

private slots:
    /** @brief 处理队列中的下一个事务 */
    void processNextTransaction();

private:
    /** @brief 执行单个SPI事务 @param transaction 事务参数 */
    void executeSpiTransaction(const BridgeTransaction& transaction);
    /** @brief 执行单个I2C事务 @param transaction 事务参数 */
    void executeI2cTransaction(const BridgeTransaction& transaction);

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
