/**
 * @file MqttSubscriptionPanel.h
 * @brief MQTT订阅管理面板 — 管理MQTT主题订阅/取消订阅操作
 *
 * 职责: 展示当前订阅列表，提供主题输入和QoS选择，
 * 通过信号通知上层执行订阅/取消订阅操作。
 */
#ifndef MQTTSUBSCRIPTIONPANEL_H
#define MQTTSUBSCRIPTIONPANEL_H

#include <QWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVariantList>

/**
 * @brief MQTT订阅管理面板
 *
 * 包含订阅列表树、主题输入框、QoS选择和订阅/取消按钮。
 * subscriptions()返回当前所有订阅信息。
 */
class MqttSubscriptionPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造订阅管理面板
     * @param parent 父控件
     */
    explicit MqttSubscriptionPanel(QWidget* parent = nullptr);

    /**
     * @brief 添加订阅项
     * @param topic 订阅主题
     * @param qos 服务质量等级
     */
    void addSubscription(const QString& topic, int qos);

    /**
     * @brief 获取所有当前订阅
     * @return QVariantList，每项包含topic和qos字段
     */
    QVariantList subscriptions() const;

signals:
    /** @brief 用户请求订阅主题 */
    void subscribeRequested(const QString& topic, int qos);

    /** @brief 用户请求取消订阅 */
    void unsubscribeRequested(const QString& topic);

private slots:
    /** @brief 订阅按钮点击处理 */
    void onSubscribeClicked();

    /** @brief 取消订阅按钮点击处理 */
    void onUnsubscribeClicked();

private:
    /** @brief 订阅列表树控件 */
    QTreeWidget* m_subTree;

    /** @brief 主题输入框 */
    QLineEdit* m_topicEdit;

    /** @brief QoS选择下拉框 */
    QComboBox* m_qosCombo;

    /** @brief 订阅按钮 */
    QPushButton* m_subBtn;

    /** @brief 取消订阅按钮 */
    QPushButton* m_unsubBtn;
};

#endif // MQTTSUBSCRIPTIONPANEL_H
