/**
 * @file CanConfigPanel.h
 * @brief CAN配置面板 — 提供CAN适配器选择、波特率、CAN-FD模式、位时序等配置界面
 *
 * 职责: 收集CAN总线连接参数(适配器/波特率/CAN-FD开关/采样点/SJW/帧过滤)，
 * 通过config()供上层获取配置。支持连接/断开切换和设置持久化。
 */
#ifndef CANCONFIGPANEL_H
#define CANCONFIGPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSettings>
#include <QVariantMap>

/**
 * @brief CAN总线连接配置面板
 *
 * 提供适配器选择、波特率设置、CAN-FD模式开关、位时序配置(采样点/SJW)、
 * 帧过滤器配置(ID+掩码)。config()返回QVariantMap供上层使用。
 */
class CanConfigPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造CAN配置面板
     * @param parent 父控件
     */
    explicit CanConfigPanel(QWidget* parent = nullptr);

    /**
     * @brief 获取当前配置参数
     * @return QVariantMap，包含adapter/bitrate/canFd/samplePoint/sjw/filterId/filterMask
     */
    QVariantMap config() const;

    /** @brief 获取累计配置变更次数 */
    quint64 totalConfigChanges() const { return m_totalConfigChanges; }

    /** @brief 获取累计总线重置次数 */
    quint64 totalBusResets() const { return m_totalBusResets; }

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
    /** @brief 用户点击连接按钮 */
    void connectRequested();
    /** @brief 用户点击断开按钮 */
    void disconnectRequested();

private:
    QComboBox* m_adapterCombo;         ///< CAN适配器选择下拉框
    QComboBox* m_bitrateCombo;         ///< 波特率下拉框
    QCheckBox* m_canFdCheck;           ///< CAN-FD模式复选框
    QPushButton* m_connectBtn;         ///< 连接/断开按钮
    QLabel* m_statusLabel;             ///< 状态标签
    QDoubleSpinBox* m_samplePointSpin; ///< 采样点(0.5~0.9)输入框
    QSpinBox* m_sjwSpin;              ///< SJW(1~4)输入框
    QLineEdit* m_filterIdEdit;        ///< 过滤器ID输入框
    QLineEdit* m_filterMaskEdit;      ///< 过滤器掩码输入框
    QCheckBox* m_filterExtCheck;      ///< 过滤器扩展帧复选框
    bool m_connected = false;         ///< 当前是否已连接

    // ---- 统计计数器 ----
    quint64 m_totalConfigChanges = 0;  ///< 累计配置变更次数
    quint64 m_totalBusResets = 0;      ///< 累计总线重置次数
};

#endif // CANCONFIGPANEL_H
