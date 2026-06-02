/**
 * @file BleConfigPanel.h
 * @brief BLE配置面板 — 提供BLE设备扫描、选择和连接配置界面
 *
 * 职责: 设备扫描触发、目标地址输入、已发现设备列表展示，
 * 收集配置参数后通过config()供上层获取。
 */
#ifndef BLECONFIGPANEL_H
#define BLECONFIGPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVariantMap>

/**
 * @brief BLE连接配置面板
 *
 * 提供设备扫描、地址输入和连接按钮。
 * config()返回的QVariantMap包含address字段。
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
     * @return QVariantMap，包含 address 字段
     */
    QVariantMap config() const;

signals:
    /** @brief 用户点击扫描按钮 */
    void scanRequested();

    /** @brief 用户点击连接按钮 */
    void connectRequested();

private:
    /** @brief 已发现设备下拉框 */
    QComboBox* m_deviceCombo;

    /** @brief 手动输入BLE地址 */
    QLineEdit* m_addressEdit;

    /** @brief 扫描按钮 */
    QPushButton* m_scanBtn;

    /** @brief 连接按钮 */
    QPushButton* m_connectBtn;
};

#endif // BLECONFIGPANEL_H
