/**
 * @file MqttSubscriptionPanel.h
 * @brief MQTT订阅管理面板 — 管理MQTT主题订阅/取消订阅操作
 *
 * 职责: 展示当前订阅列表，提供主题输入和QoS选择，
 * 支持右键上下文菜单取消订阅。
 */
#ifndef MQTTSUBSCRIPTIONPANEL_H
#define MQTTSUBSCRIPTIONPANEL_H

#include <QWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QVariantList>

/**
 * @brief MQTT订阅管理面板
 *
 * 包含订阅列表树、主题输入框、QoS选择和订阅/取消按钮。
 * 支持右键上下文菜单取消订阅。subscriptions()返回当前所有订阅信息。
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

    /** @brief 获取累计订阅操作次数 @return 订阅总次数 */
    quint64 totalSubscriptions() const { return m_totalSubscriptions; }

    /** @brief 获取累计取消订阅操作次数 @return 取消订阅总次数 */
    quint64 totalUnsubscriptions() const { return m_totalUnsubscriptions; }

    /** @brief 获取累计主题编辑次数(输入框回车等) @return 编辑总次数 */
    quint64 totalTopicEdits() const { return m_totalTopicEdits; }

    /** @brief 获取累计订阅失败次数(空主题等) @return 失败总次数 */
    quint64 totalSubscriptionFailures() const { return m_totalSubscriptionFailures; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

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

    /** @brief 右键上下文菜单 */
    void onCustomContextMenu(const QPoint& pos);

private:
    QTreeWidget* m_subTree;                ///< 订阅列表树控件
    QLineEdit* m_topicEdit;                ///< 主题输入框
    QComboBox* m_qosCombo;                 ///< QoS选择下拉框
    QPushButton* m_subBtn;                 ///< 订阅按钮
    QPushButton* m_unsubBtn;               ///< 取消订阅按钮
    QMenu* m_contextMenu;                  ///< 右键菜单
    QAction* m_unsubAction;                ///< 取消订阅动作

    // ---- 统计计数器 ----
    quint64 m_totalSubscriptions = 0;     ///< 累计订阅操作次数
    quint64 m_totalUnsubscriptions = 0;   ///< 累计取消订阅操作次数
    quint64 m_totalTopicEdits = 0;        ///< 累计主题编辑次数
    quint64 m_totalSubscriptionFailures = 0; ///< 累计订阅失败次数
};

#endif // MQTTSUBSCRIPTIONPANEL_H
