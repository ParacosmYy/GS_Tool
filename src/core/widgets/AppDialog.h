/**
 * @file AppDialog.h
 * @brief 统一弹窗组件 - 替代QMessageBox的自定义对话框
 *
 * 提供确认、警告、错误、信息四种弹窗类型，统一应用内所有对话框风格。
 * 支持自定义按钮文本、图标、详情文本展开。
 *
 * 设计原则:
 *   - 禁止QMessageBox: 所有弹窗必须使用 AppDialog
 *   - 主题适配: 颜色从QSS获取，不硬编码
 *   - 国际化: 所有文字使用tr()
 *   - 动画: 弹窗出现/消失有淡入淡出效果
 *
 * 使用示例:
 *   AppDialog::confirm(tr("确认删除"), tr("确定要删除所有数据吗？"), this)
 *       .then([this]() { doDelete(); });
 *
 * 协作关系:
 *   - ThemeManager: 获取语义颜色
 *   - IconManager: 获取弹窗图标
 *   - 所有模块: 统一弹窗入口
 */
#ifndef APPDIALOG_H
#define APPDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <functional>

/**
 * @brief 统一弹窗组件 - 替代QMessageBox的自定义对话框
 *
 * 提供 confirm/warning/error/information 四种静态方法，
 * 返回 DialogPromise 支持链式调用。
 */
class AppDialog : public QDialog {
    Q_OBJECT

public:
    /** @brief 弹窗类型 */
    enum class Type {
        Confirm,     ///< 确认对话框 (蓝绿色)
        Warning,     ///< 警告对话框 (橙色)
        Error,       ///< 错误对话框 (红色)
        Information  ///< 信息对话框 (蓝色)
    };
    Q_ENUM(Type)

    /** @brief 弹窗结果回调 Promise */
    class DialogPromise {
    public:
        /** @brief 用户点击确认时执行回调 */
        DialogPromise& then(std::function<void()> onAccepted);

        /** @brief 用户点击取消时执行回调 */
        DialogPromise& otherwise(std::function<void()> onRejected);

    private:
        friend class AppDialog;
        QDialog* m_dialog = nullptr;
        std::function<void()> m_onAccepted;
        std::function<void()> m_onRejected;
    };

    /**
     * @brief 显示确认对话框
     * @param title 标题
     * @param message 消息内容
     * @param parent 父窗口
     * @return DialogPromise 支持链式调用
     */
    static DialogPromise confirm(const QString& title, const QString& message,
                                  QWidget* parent = nullptr);

    /**
     * @brief 显示警告对话框
     * @param title 标题
     * @param message 消息内容
     * @param parent 父窗口
     * @return DialogPromise
     */
    static DialogPromise warning(const QString& title, const QString& message,
                                  QWidget* parent = nullptr);

    /**
     * @brief 显示错误对话框
     * @param title 标题
     * @param message 消息内容
     * @param parent 父窗口
     * @return DialogPromise
     */
    static DialogPromise error(const QString& title, const QString& message,
                                QWidget* parent = nullptr);

    /**
     * @brief 显示信息对话框
     * @param title 标题
     * @param message 消息内容
     * @param parent 父窗口
     * @return DialogPromise
     */
    static DialogPromise information(const QString& title, const QString& message,
                                      QWidget* parent = nullptr);

    /**
     * @brief 显示自定义对话框
     * @param type 弹窗类型
     * @param title 标题
     * @param message 消息内容
     * @param confirmText 确认按钮文本 (空则使用默认)
     * @param cancelText 取消按钮文本 (空则使用默认)
     * @param parent 父窗口
     * @return DialogPromise
     */
    static DialogPromise show(Type type, const QString& title, const QString& message,
                               const QString& confirmText = QString(),
                               const QString& cancelText = QString(),
                               QWidget* parent = nullptr);

private:
    explicit AppDialog(Type type, const QString& title, const QString& message,
                        const QString& confirmText, const QString& cancelText,
                        QWidget* parent = nullptr);

    void setupUI();
    void applyStyle();
    void showEvent(QShowEvent* event) override;

    Type m_type;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_messageLabel = nullptr;
    QPushButton* m_confirmBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;
    QString m_confirmText;
    QString m_cancelText;
};

#endif // APPDIALOG_H
