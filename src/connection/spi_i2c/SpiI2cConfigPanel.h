/**
 * @file SpiI2cConfigPanel.h
 * @brief SPI/I2C配置面板 - 配置SPI或I2C总线参数的UI
 *
 * 职责:
 *   1. 提供SPI/I2C模式切换
 *   2. 配置适配器、时钟频率、SPI模式等参数
 *   3. 提供连接/断开操作按钮
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
#include <QVBoxLayout>
#include <QVariant>

/**
 * @brief SPI/I2C配置面板UI
 *
 * 根据当前模式(SPI/I2C)动态显示不同的配置选项。
 * SPI模式: 时钟频率、SPI模式、片选引脚
 * I2C模式: 设备地址、时钟频率
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

    /**
     * @brief 设置当前模式(SPI或I2C)
     * @param mode "spi" 或 "i2c"
     */
    void setMode(const QString& mode);

private slots:
    /** @brief 连接按钮点击 */
    void onConnectClicked();

private:
    /** @brief 初始化UI布局 */
    void setupUi();

    /** @brief 初始化信号连接 */
    void setupConnections();

    /** @brief 根据模式更新UI可见性 */
    void updateModeVisibility();

    // ---- UI控件 ----
    QComboBox* m_adapterCombo = nullptr;            ///< 适配器选择下拉框
    QSpinBox* m_clockSpin = nullptr;                ///< 时钟频率设置
    QComboBox* m_modeCombo = nullptr;               ///< SPI模式/I2C模式下拉框
    QPushButton* m_connectBtn = nullptr;            ///< 连接/断开按钮

    QString m_currentMode = "spi";                  ///< 当前模式("spi"/"i2c")
};

#endif // SPII2CCONFIGPANEL_H
