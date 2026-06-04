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

    /** @brief 重写exec，递增对话框显示计数 @return 对话框结果 */
    int exec() override;

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

    // ── 统计计数器 Getter ──

    /** @brief 获取对话框总显示次数 @return exec()调用累计次数 */
    quint64 totalDialogShows() const { return m_totalDialogShows; }

    /** @brief 获取用户确认连接总次数 @return 点击"连接"按钮的累计次数 */
    quint64 totalDialogAccepts() const { return m_totalDialogAccepts; }

    /** @brief 获取用户取消对话框总次数 @return 点击"取消"按钮的累计次数 */
    quint64 totalDialogCancels() const { return m_totalDialogCancels; }

    /** @brief 获取连接类型切换总次数 @return 下拉框切换连接类型的累计次数 */
    quint64 totalTypeSwitches() const { return m_totalTypeSwitches; }

    /** @brief 重置所有统计计数器为零 */
    void resetStats();

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

    // ── 统计计数器 ──
    quint64 m_totalDialogShows = 0;   ///< 对话框总显示次数
    quint64 m_totalDialogAccepts = 0; ///< 用户确认连接总次数
    quint64 m_totalDialogCancels = 0; ///< 用户取消对话框总次数
    quint64 m_totalTypeSwitches = 0;  ///< 连接类型切换总次数
};

#endif // CONNECTIONQUICKDIALOG_H
