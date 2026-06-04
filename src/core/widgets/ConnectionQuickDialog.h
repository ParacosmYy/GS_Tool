/**
 * @file ConnectionQuickDialog.h
 * @brief 快速连接对话框 - 提供 Ctrl+N 的轻量连接参数入口
 */

#ifndef CONNECTIONQUICKDIALOG_H
#define CONNECTIONQUICKDIALOG_H

#include <QDialog>
#include <QVariantMap>
#include "shared/AppConstants.h"

class QComboBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QSpinBox;

/**
 * @brief 快速连接对话框
 *
 * 只负责收集连接类型和基础参数，不负责实际创建连接。
 */
class ConnectionQuickDialog : public QDialog {
    Q_OBJECT

public:
    /** @brief 构造快速连接对话框 @param parent 父窗口 */
    explicit ConnectionQuickDialog(QWidget* parent = nullptr);

    /** @brief 获取当前选中的连接类型 @return 连接类型 */
    ConnectionType selectedType() const;

    /**
     * @brief 获取当前表单参数
     *
     * 返回值按所选连接类型填充对应的 configure() 键值对，
     * 可直接传给连接控制器。
     *
     * @return 参数映射
     */
    QVariantMap params() const;

private:
    void setupUi();
    void setupConnections();
    void updateTypeUi();
    void applyDefaults(ConnectionType type);
    QVariantMap buildParams(ConnectionType type) const;
    static void setRowVisible(QLabel* label, QWidget* field, bool visible);

    QComboBox* m_typeCombo = nullptr;
    QLabel* m_hostLabel = nullptr;
    QLineEdit* m_hostEdit = nullptr;
    QLabel* m_portLabel = nullptr;
    QSpinBox* m_portSpin = nullptr;
    QLabel* m_urlLabel = nullptr;
    QLineEdit* m_urlEdit = nullptr;
    QLabel* m_localPortLabel = nullptr;
    QSpinBox* m_localPortSpin = nullptr;
    QLabel* m_remoteHostLabel = nullptr;
    QLineEdit* m_remoteHostEdit = nullptr;
    QLabel* m_remotePortLabel = nullptr;
    QSpinBox* m_remotePortSpin = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;
};

#endif // CONNECTIONQUICKDIALOG_H
