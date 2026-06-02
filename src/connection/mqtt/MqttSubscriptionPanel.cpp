/**
 * @file MqttSubscriptionPanel.cpp
 * @brief MQTT订阅管理面板实现
 */

#include "connection/mqtt/MqttSubscriptionPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

MqttSubscriptionPanel::MqttSubscriptionPanel(QWidget* parent)
    : QWidget(parent)
    , m_subTree(new QTreeWidget(this))
    , m_topicEdit(new QLineEdit(this))
    , m_qosCombo(new QComboBox(this))
    , m_subBtn(new QPushButton(tr("订阅"), this))
    , m_unsubBtn(new QPushButton(tr("取消订阅"), this))
{
    setObjectName("MqttSubscriptionPanel");

    m_subTree->setHeaderLabels({tr("主题"), tr("QoS")});
    m_subTree->setRootIsDecorated(false);
    m_subTree->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_topicEdit->setPlaceholderText(tr("输入主题，如 sensor/#"));

    m_qosCombo->addItem("0", 0);
    m_qosCombo->addItem("1", 1);
    m_qosCombo->addItem("2", 2);

    auto inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_topicEdit, 1);
    inputLayout->addWidget(m_qosCombo);
    inputLayout->addWidget(m_subBtn);
    inputLayout->addWidget(m_unsubBtn);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_subTree, 1);

    connect(m_subBtn, &QPushButton::clicked, this, &MqttSubscriptionPanel::onSubscribeClicked);
    connect(m_unsubBtn, &QPushButton::clicked, this, &MqttSubscriptionPanel::onUnsubscribeClicked);
}

void MqttSubscriptionPanel::addSubscription(const QString& topic, int qos)
{
    auto item = new QTreeWidgetItem(m_subTree);
    item->setText(0, topic);
    item->setText(1, QString::number(qos));
    m_subTree->addTopLevelItem(item);
}

QVariantList MqttSubscriptionPanel::subscriptions() const
{
    QVariantList result;
    for (int i = 0; i < m_subTree->topLevelItemCount(); ++i) {
        auto item = m_subTree->topLevelItem(i);
        QVariantMap sub;
        sub["topic"] = item->text(0);
        sub["qos"] = item->text(1).toInt();
        result.append(sub);
    }
    return result;
}

void MqttSubscriptionPanel::onSubscribeClicked()
{
    const QString topic = m_topicEdit->text().trimmed();
    if (topic.isEmpty()) return;
    const int qos = m_qosCombo->currentData().toInt();
    addSubscription(topic, qos);
    emit subscribeRequested(topic, qos);
    m_topicEdit->clear();
}

void MqttSubscriptionPanel::onUnsubscribeClicked()
{
    auto selected = m_subTree->selectedItems();
    for (auto* item : selected) {
        const QString topic = item->text(0);
        emit unsubscribeRequested(topic);
        delete item;
    }
}
