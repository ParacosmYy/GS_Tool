/**
 * @file CanConfigPanel.h
 * @brief CAN配置面板 — 提供CAN适配器选择、波特率、CAN-FD模式等配置界面
 *
 * 职责: 收集CAN总线连接参数(适配器/波特率/CAN-FD开关)，
 * 通过config()供上层获取配置。支持连接/断开切换。
 */
#ifndef CANCONFIGPANEL_H
#define CANCONFIGPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QVariantMap>

/**
 * @brief CAN总线连接配置面板
 *
 * 提供适配器选择、波特率设置和CAN-FD模式开关。
 * config()返回QVariantMap，包含adapter/bitrate/canFd字段。
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
     * @return QVariantMap，包含adapter/bitrate/canFd
     */
    QVariantMap config() const;

    /**
     * @brief 设置连接状态(更新按钮文本和状态标签)
     * @param connected true=已连接
     */
    void setConnected(bool connected);

signals:
    /** @brief 用户点击连接/断开按钮 */
    void connectRequested();
    void disconnectRequested();

private:
    /** @brief CAN适配器选择下拉框 */
    QComboBox* m_adapterCombo;

    /** @brief 波特率下拉框 */
    QComboBox* m_bitrateCombo;

    /** @brief CAN-FD模式复选框 */
    QCheckBox* m_canFdCheck;

    /** @brief 连接/断开按钮 */
    QPushButton* m_connectBtn;

    /** @brief 状态标签 */
    QLabel* m_statusLabel;

    /** @brief 当前是否已连接 */
    bool m_connected = false;
};

#endif // CANCONFIGPANEL_H
