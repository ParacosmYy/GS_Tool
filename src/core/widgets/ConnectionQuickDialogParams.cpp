/**
 * @file ConnectionQuickDialogParams.cpp
 * @brief 快速连接对话框 - 参数构建与类型适配方法实现
 *
 * 从 ConnectionQuickDialog.cpp 拆分而来，包含:
 *   - selectedType():    获取当前选中的连接类型
 *   - params():          获取当前表单参数映射
 *   - applyDefaults():   按连接类型应用默认值
 *   - buildParams():     按连接类型构建参数映射
 *   - setRowVisible():   统一设置一行控件的显示/隐藏状态
 *   - updateTypeUi():    根据当前类型刷新默认值和字段可见性
 *
 * 拆分原因: ConnectionQuickDialog.cpp 中UI构建(setupUi/setupConnections)与
 * 参数逻辑(applyDefaults/buildParams/updateTypeUi)属于不同关注点，
 * 将参数逻辑独立可便于添加新的连接类型和预设值。
 */

#include "core/widgets/ConnectionQuickDialog.h"

#include "core/connect/ConnectionPresetBuilder.h"

/** @brief 获取当前选中的连接类型 @return 连接类型 */
ConnectionType ConnectionQuickDialog::selectedType() const
{
    return static_cast<ConnectionType>(m_typeCombo->currentData().toInt());
}

/** @brief 获取当前表单参数 @return 连接参数映射 */
QVariantMap ConnectionQuickDialog::params() const
{
    return buildParams(selectedType());
}

/** @brief 应用当前类型的默认值 */
void ConnectionQuickDialog::applyDefaults(ConnectionType type)
{
    const QVariantMap preset = ConnectionPresetBuilder::build(type);

    m_hostEdit->clear();
    m_portSpin->setValue(0);
    m_urlEdit->clear();
    m_localPortSpin->setValue(0);
    m_remoteHostEdit->clear();
    m_remotePortSpin->setValue(0);

    if (type == ConnectionType::TcpClient) {
        m_hostEdit->setText(preset.value("host", QStringLiteral("127.0.0.1")).toString());
        m_portSpin->setValue(preset.value("port", 8080).toInt());
    } else if (type == ConnectionType::TcpServer) {
        m_hostEdit->setText(QStringLiteral("0.0.0.0"));
        m_portSpin->setValue(preset.value("port", 8080).toInt());
    } else if (type == ConnectionType::Udp) {
        m_localPortSpin->setValue(preset.value("localPort", 8888).toInt());
        m_remoteHostEdit->setText(preset.value("remoteHost", QStringLiteral("127.0.0.1")).toString());
        m_remotePortSpin->setValue(preset.value("remotePort", 8080).toInt());
    } else if (type == ConnectionType::WebSocket) {
        m_urlEdit->setText(preset.value("url", QStringLiteral("ws://127.0.0.1:8080")).toString());
    } else if (type == ConnectionType::Mqtt) {
        m_hostEdit->setText(preset.value("host", QStringLiteral("127.0.0.1")).toString());
        m_portSpin->setValue(preset.value("port", 1883).toInt());
    } else if (type == ConnectionType::Tls) {
        m_hostEdit->setText(preset.value("host", QStringLiteral("127.0.0.1")).toString());
        m_portSpin->setValue(preset.value("port", 443).toInt());
    }
}

/** @brief 按连接类型构建参数映射 @param type 连接类型 @return 可直接传给连接控制器的参数 */
QVariantMap ConnectionQuickDialog::buildParams(ConnectionType type) const
{
    QVariantMap params;

    if (type == ConnectionType::TcpClient) {
        params["host"] = m_hostEdit->text().trimmed();
        params["port"] = m_portSpin->value();
    } else if (type == ConnectionType::TcpServer) {
        params["host"] = m_hostEdit->text().trimmed();
        params["port"] = m_portSpin->value();
    } else if (type == ConnectionType::Udp) {
        params["localPort"] = m_localPortSpin->value();
        params["remoteHost"] = m_remoteHostEdit->text().trimmed();
        params["remotePort"] = m_remotePortSpin->value();
    } else if (type == ConnectionType::WebSocket) {
        params["url"] = m_urlEdit->text().trimmed();
    } else if (type == ConnectionType::Mqtt) {
        params["host"] = m_hostEdit->text().trimmed();
        params["port"] = m_portSpin->value();
    } else if (type == ConnectionType::Tls) {
        params["host"] = m_hostEdit->text().trimmed();
        params["port"] = m_portSpin->value();
    }

    return params;
}

/** @brief 统一设置一行控件的显示/隐藏状态 @param label 标签控件 @param field 输入控件 @param visible 是否可见 */
void ConnectionQuickDialog::setRowVisible(QLabel* label, QWidget* field, bool visible)
{
    if (label) {
        label->setVisible(visible);
    }
    if (field) {
        field->setVisible(visible);
    }
}

/** @brief 根据当前类型刷新默认值和字段可见性 */
void ConnectionQuickDialog::updateTypeUi()
{
    const ConnectionType type = selectedType();
    applyDefaults(type);

    const bool useHostPort = type == ConnectionType::TcpClient
        || type == ConnectionType::TcpServer
        || type == ConnectionType::Mqtt
        || type == ConnectionType::Tls;
    const bool useUrl = type == ConnectionType::WebSocket;
    const bool useUdp = type == ConnectionType::Udp;

    setRowVisible(m_hostLabel, m_hostEdit, useHostPort);
    setRowVisible(m_portLabel, m_portSpin, useHostPort);
    setRowVisible(m_urlLabel, m_urlEdit, useUrl);
    setRowVisible(m_localPortLabel, m_localPortSpin, useUdp);
    setRowVisible(m_remoteHostLabel, m_remoteHostEdit, useUdp);
    setRowVisible(m_remotePortLabel, m_remotePortSpin, useUdp);

    if (type == ConnectionType::TcpClient || type == ConnectionType::Mqtt || type == ConnectionType::Tls) {
        m_hostLabel->setText(tr("主机:"));
    } else if (type == ConnectionType::TcpServer) {
        m_hostLabel->setText(tr("监听地址:"));
    } else {
        m_hostLabel->setText(tr("地址:"));
    }

    m_portLabel->setText(tr("端口:"));
    m_urlLabel->setText(tr("URL:"));
    m_localPortLabel->setText(tr("本地端口:"));
    m_remoteHostLabel->setText(tr("远程主机:"));
    m_remotePortLabel->setText(tr("远程端口:"));
}
