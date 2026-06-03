/**
 * @file UsbConfigPanel.h
 * @brief USB配置面板 — 配置USB连接参数
 *
 * 提供设备选择、VID/PID输入、接口编号和连接按钮。
 * 集成UsbDeviceDetector实现设备自动扫描。
 */
#ifndef USB_CONFIG_PANEL_H
#define USB_CONFIG_PANEL_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QSettings>

class UsbDeviceDetector;

/**
 * @brief USB连接配置面板控件
 * 配置USB设备的VID、PID、接口编号等连接参数。
 */
class UsbConfigPanel : public QWidget {
    Q_OBJECT

public:
    explicit UsbConfigPanel(QWidget* parent = nullptr);

    /**
     * @brief 设置设备检测器
     * @param detector 检测器实例
     */
    void setDetector(UsbDeviceDetector* detector);

    /** @brief 获取累计设备刷新次数 */
    quint64 totalDeviceRefreshes() const { return m_totalDeviceRefreshes; }

    /** @brief 获取累计配置变更次数 */
    quint64 totalConfigChanges() const { return m_totalConfigChanges; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

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
    /** @brief 用户请求连接 */
    void connectRequested(quint16 vid, quint16 pid, int interface);

    /** @brief 用户请求断开 */
    void disconnectRequested();

private slots:
    /** @brief 扫描设备按钮点击 */
    void onScanClicked();

    /** @brief 设备选择变更 */
    void onDeviceChanged(int index);

    /** @brief 连接/断开按钮点击 */
    void onConnectClicked();

private:
    QComboBox*  m_deviceCombo    = nullptr; ///< 设备选择下拉框
    QSpinBox*   m_vidSpin        = nullptr; ///< VID输入 (十六进制)
    QSpinBox*   m_pidSpin        = nullptr; ///< PID输入 (十六进制)
    QSpinBox*   m_interfaceSpin  = nullptr; ///< 接口编号
    QPushButton* m_connectBtn    = nullptr; ///< 连接/断开按钮
    QPushButton* m_scanBtn       = nullptr; ///< 扫描设备按钮
    QLabel*     m_statusLabel    = nullptr; ///< 状态标签
    UsbDeviceDetector* m_detector = nullptr; ///< 设备检测器
    bool m_connected = false;                ///< 当前连接状态

    // ---- 统计计数器 ----
    quint64 m_totalDeviceRefreshes = 0;  ///< 累计设备刷新次数
    quint64 m_totalConfigChanges = 0;    ///< 累计配置变更次数
};

#endif // USB_CONFIG_PANEL_H
