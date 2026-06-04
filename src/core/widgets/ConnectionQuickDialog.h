/**
 * @file ConnectionQuickDialog.h
 * @brief 快速连接对话框 - 提供 Ctrl+N 的轻量连接参数入口
 *
 * 这个对话框只负责收集连接类型和最少必要参数，不负责真正创建连接。
 * 上层可以通过 selectedType() 和 params() 直接把结果交给连接控制器。
 */

#ifndef CONNECTIONQUICKDIALOG_H
#define CONNECTIONQUICKDIALOG_H

#include <QDialog>
#include <QVariantMap>
#include "shared/AppConstants.h"

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QSpinBox;

/**
 * @brief 快速连接对话框
 *
 * 当前版本采用共享字段的方式覆盖常见网络连接类型:
 * TcpClient、TcpServer、Udp、WebSocket、Mqtt、Tls。
 * 通过 type 切换时刷新默认值和字段可见性，保持界面轻量可复用。
 */
class ConnectionQuickDialog : public QDialog {
    Q_OBJECT

public:
    /** @brief 构造快速连接对话框 @param parent 父窗口 */
    explicit ConnectionQuickDialog(QWidget* parent = nullptr);

    /** @brief 设置当前选中的连接类型 @param type 连接类型 */
    void setSelectedType(ConnectionType type);

    /** @brief 获取当前选中的连接类型 @return 连接类型 */
    ConnectionType selectedType() const;

    /**
     * @brief 获取当前表单参数
     *
     * 返回值按所选连接类型填充对应的 configure() 键值对，
     * 可直接传给 ConnectionController::connectNetwork(type, params)。
     *
     * @return 参数映射
     */
    QVariantMap params() const;

private:
    void setupUi();
    void setupConnections();
    void refreshTypeUi();
    void applyTypePreset(ConnectionType type);
    void updateFieldVisibility(ConnectionType type);
    void updateFieldLabels(ConnectionType type);
    void updateFieldPlaceholders(ConnectionType type);
    QString validateCurrentInput() const;
    QVariantMap buildParams(ConnectionType type) const;
    static QString defaultHint(ConnectionType type);
    static QString normalizeWebSocketUrl(const QString& text);
    static void setRowVisible(QWidget* label, QWidget* field, bool visible);

    QComboBox* m_typeCombo = nullptr;
    QLabel* m_typeLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_descriptionLabel = nullptr;
    QLabel* m_hostLabel = nullptr;
    QLabel* m_portLabel = nullptr;
    QLabel* m_localPortLabel = nullptr;
    QLabel* m_extraLabel = nullptr;
    QLabel* m_clientIdLabel = nullptr;
    QLabel* m_usernameLabel = nullptr;
    QLabel* m_passwordLabel = nullptr;
    QLabel* m_keepAliveLabel = nullptr;
    QLabel* m_hintLabel = nullptr;
    QLineEdit* m_hostEdit = nullptr;
    QSpinBox* m_portSpin = nullptr;
    QSpinBox* m_localPortSpin = nullptr;
    QLineEdit* m_extraEdit = nullptr;
    QLineEdit* m_clientIdEdit = nullptr;
    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QSpinBox* m_keepAliveSpin = nullptr;
    QCheckBox* m_cleanSessionCheck = nullptr;
    QCheckBox* m_broadcastCheck = nullptr;
    QCheckBox* m_peerVerifyCheck = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;
};

#endif // CONNECTIONQUICKDIALOG_H
