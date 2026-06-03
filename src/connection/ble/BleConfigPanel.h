/**
 * @file BleConfigPanel.h
 * @brief BLE配置面板 — 提供BLE设备扫描、选择和连接配置界面
 *
 * 职责: 设备扫描触发、目标地址输入、已发现设备列表展示，
 * 连接状态显示，收集配置参数后通过config()供上层获取。
 */
#ifndef BLECONFIGPANEL_H
#define BLECONFIGPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <QVariantMap>

class BleScanner;

/**
 * @brief BLE连接配置面板
 *
 * 提供设备扫描、地址输入、连接按钮和状态显示。
 * config()返回的QVariantMap包含address和deviceName字段。
 */
class BleConfigPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造BLE配置面板
     * @param parent 父控件
     */
    explicit BleConfigPanel(QWidget* parent = nullptr);

    /**
     * @brief 获取当前配置参数
     * @return QVariantMap，包含 address 和 deviceName 字段
     */
    QVariantMap config() const;

    /**
     * @brief 设置扫描器实例
     * @param scanner BLE扫描器对象
     */
    void setScanner(BleScanner* scanner);

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
    /** @brief 用户点击扫描按钮 */
    void scanRequested();

    /** @brief 用户点击连接按钮 */
    void connectRequested();

private slots:
    /** @brief 处理发现的新设备 */
    void onDeviceFound(const QVariantMap& device);

    /** @brief 扫描完成处理 */
    void onScanFinished();

    /** @brief 设备下拉框选择变化处理 */
    void onDeviceSelected(int index);

private:
    /** @brief 已发现设备下拉框 */
    QComboBox* m_deviceCombo;

    /** @brief 手动输入BLE地址 */
    QLineEdit* m_addressEdit;

    /** @brief 扫描按钮 */
    QPushButton* m_scanBtn;

    /** @brief 连接按钮 */
    QPushButton* m_connectBtn;

    /** @brief 连接状态标签 */
    QLabel* m_statusLabel;

    /** @brief 已发现的设备数据列表（与下拉框同步） */
    QVariantList m_deviceList;

    /** @brief 扫描器实例 */
    BleScanner* m_scanner = nullptr;
};

#endif // BLECONFIGPANEL_H
