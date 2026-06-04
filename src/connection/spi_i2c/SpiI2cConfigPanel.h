/**
 * @file SpiI2cConfigPanel.h
 * @brief SPI/I2C配置面板 - 配置SPI或I2C总线参数的UI
 *
 * 职责:
 *   1. 提供SPI/I2C模式切换
 *   2. 配置适配器、时钟频率、SPI模式等参数
 *   3. 提供连接/断开操作按钮
 *   4. 根据模式动态显示/隐藏相关控件
 *
 * 协作关系:
 *   - SpiConnection: SPI连接配置
 *   - I2cConnection: I2C连接配置
 */

#ifndef SPII2CCONFIGPANEL_H
#define SPII2CCONFIGPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <QVBoxLayout>
#include <QVariant>
#include <QGroupBox>

class IConnection;

/**
 * @brief SPI/I2C配置面板UI
 *
 * 根据当前模式(SPI/I2C)动态显示不同的配置选项。
 * SPI模式: 时钟频率、SPI模式、片选引脚
 * I2C模式: 设备地址(十六进制)、时钟频率
 */
class SpiI2cConfigPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造函数
     * @param parent 父控件
     */
    explicit SpiI2cConfigPanel(QWidget* parent = nullptr);

    /**
     * @brief 获取当前配置参数
     * @return 配置键值对(QVariantMap)
     */
    QVariantMap config() const;

    /** @brief 获取累计传输操作次数 @return 传输总次数 */
    quint64 totalTransfers() const { return m_totalTransfers; }

    /** @brief 获取累计配置变更次数 @return 配置变更总次数 */
    quint64 totalConfigChanges() const { return m_totalConfigChanges; }

    /** @brief 获取累计模式切换次数 @return 模式切换总次数 */
    quint64 totalModeChanges() const { return m_totalModeChanges; }

    /** @brief 获取累计时钟频率变更次数 @return 频率变更总次数 */
    quint64 totalSpeedChanges() const { return m_totalSpeedChanges; }

    /** @brief 获取累计传输错误次数 @return 传输错误总次数 */
    quint64 totalTransferErrors() const { return m_totalTransferErrors; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

    /**
     * @brief 设置当前模式(SPI或I2C)
     * @param mode "spi" 或 "i2c"
     */
    void setMode(const QString& mode);

    /**
     * @brief 设置连接状态(更新按钮文本和状态标签)
     * @param connected true=已连接
     */
    void setConnected(bool connected);

    /**
     * @brief 保存配置到QSettings
     * @param settings QSettings对象
     */
    void saveSettings(QSettings& settings) const;

    /**
     * @brief 从QSettings加载配置
     * @param settings QSettings对象
     */
    void loadSettings(QSettings& settings);

signals:
    /** @brief 请求连接信号，携带配置参数 */
    void connectRequested(const QVariantMap& config);

    /** @brief 请求断开连接信号 */
    void disconnectRequested();

private slots:
    /** @brief 连接按钮点击 */
    void onConnectClicked();

    /** @brief 模式切换(SPI/I2C)回调 */
    void onModeChanged(int index);

private:
    /** @brief 初始化UI布局 */
    void setupUi();

    /** @brief 初始化信号连接 */
    void setupConnections();

    /** @brief 根据模式更新UI可见性 */
    void updateModeVisibility();

    // ---- 通用控件 ----
    QComboBox* m_busModeCombo = nullptr;             ///< 总线模式: SPI/I2C
    QComboBox* m_adapterCombo = nullptr;             ///< 适配器选择下拉框
    QSpinBox* m_clockSpin = nullptr;                 ///< 时钟频率设置
    QPushButton* m_connectBtn = nullptr;             ///< 连接/断开按钮
    QLabel* m_statusLabel = nullptr;                 ///< 状态显示标签

    // ---- SPI专用控件 ----
    QGroupBox* m_spiGroup = nullptr;                 ///< SPI参数分组
    QComboBox* m_spiModeCombo = nullptr;             ///< SPI模式(0-3)
    QSpinBox* m_csPinSpin = nullptr;                 ///< 片选引脚

    // ---- I2C专用控件 ----
    QGroupBox* m_i2cGroup = nullptr;                 ///< I2C参数分组
    QSpinBox* m_deviceAddrSpin = nullptr;            ///< 设备地址(十六进制)

    // ---- 状态 ----
    QString m_currentMode = "spi";                   ///< 当前模式("spi"/"i2c")
    bool m_connected = false;                        ///< 当前连接状态

    // ---- 统计计数器 ----
    quint64 m_totalTransfers = 0;       ///< 累计传输操作次数
    quint64 m_totalConfigChanges = 0;   ///< 累计配置变更次数
    quint64 m_totalModeChanges = 0;     ///< 累计模式切换次数
    quint64 m_totalSpeedChanges = 0;    ///< 累计时钟频率变更次数
    quint64 m_totalTransferErrors = 0;  ///< 累计传输错误次数
};

#endif // SPII2CCONFIGPANEL_H
