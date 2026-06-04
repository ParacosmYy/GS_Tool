/**
 * @file AppDialog.cpp
 * @brief 统一弹窗组件实现 - 替代QMessageBox的自定义对话框
 *
 * 提供确认/警告/错误/信息四种弹窗类型，支持Promise链式回调，
 * 半透明背景+圆角面板+淡入动画，所有颜色通过ThemeManager获取。
 *
 * UI构建/样式/动画方法已拆分至 AppDialogContent.cpp。
 */

#include "core/widgets/AppDialog.h"
#include "core/theme/IconManager.h"

#include <QTimer>

// ---- DialogPromise 实现 ----

/** @brief 设置弹窗确认(accepted)时的回调函数 @param onAccepted 确认回调函数 @return 当前DialogPromise引用，支持链式调用 */
AppDialog::DialogPromise& AppDialog::DialogPromise::then(std::function<void()> onAccepted)
{
    m_onAccepted = std::move(onAccepted);
    if (m_dialog) {
        QObject::connect(m_dialog, &QDialog::accepted, m_dialog, [this]() {
            if (m_onAccepted) m_onAccepted();
        });
    }
    return *this;
}

/** @brief 设置弹窗取消(rejected)时的回调函数 @param onRejected 取消回调函数 @return 当前DialogPromise引用，支持链式调用 */
AppDialog::DialogPromise& AppDialog::DialogPromise::otherwise(std::function<void()> onRejected)
{
    m_onRejected = std::move(onRejected);
    if (m_dialog) {
        QObject::connect(m_dialog, &QDialog::rejected, m_dialog, [this]() {
            if (m_onRejected) m_onRejected();
        });
    }
    return *this;
}

// ---- 静态工厂方法 ----

/** @brief 创建确认类型弹窗(带确定和取消按钮) @param title 弹窗标题 @param message 弹窗消息内容 @param parent 父控件指针 @return DialogPromise对象用于链式回调 */
AppDialog::DialogPromise AppDialog::confirm(const QString& title, const QString& message,
                                              QWidget* parent)
{
    return show(Type::Confirm, title, message, QString(), QString(), parent);
}

/** @brief 创建警告类型弹窗(带确定和取消按钮，警告色) @param title 弹窗标题 @param message 弹窗消息内容 @param parent 父控件指针 @return DialogPromise对象用于链式回调 */
AppDialog::DialogPromise AppDialog::warning(const QString& title, const QString& message,
                                              QWidget* parent)
{
    return show(Type::Warning, title, message, tr("确定"), QString(), parent);
}

/** @brief 创建错误类型弹窗(仅确定按钮，错误色) @param title 弹窗标题 @param message 弹窗消息内容 @param parent 父控件指针 @return DialogPromise对象用于链式回调 */
AppDialog::DialogPromise AppDialog::error(const QString& title, const QString& message,
                                            QWidget* parent)
{
    return show(Type::Error, title, message, tr("确定"), QString(), parent);
}

/** @brief 创建信息类型弹窗(仅确定按钮，主题色) @param title 弹窗标题 @param message 弹窗消息内容 @param parent 父控件指针 @return DialogPromise对象用于链式回调 */
AppDialog::DialogPromise AppDialog::information(const QString& title, const QString& message,
                                                  QWidget* parent)
{
    return show(Type::Information, title, message, tr("确定"), QString(), parent);
}

/** @brief 创建并显示指定类型的弹窗对话框 @param type 弹窗类型(Confirm/Warning/Error/Information) @param title 弹窗标题 @param message 消息内容 @param confirmText 确认按钮文字 @param cancelText 取消按钮文字 @param parent 父控件指针 @return DialogPromise对象用于链式回调 */
AppDialog::DialogPromise AppDialog::show(Type type, const QString& title,
                                           const QString& message,
                                           const QString& confirmText,
                                           const QString& cancelText,
                                           QWidget* parent)
{
    auto* dialog = new AppDialog(type, title, message, confirmText, cancelText, parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    DialogPromise promise;
    promise.m_dialog = dialog;

    // 连接信号
    QObject::connect(dialog, &QDialog::accepted, dialog, [promise]() {
        ++s_totalAccepted;
        if (promise.m_onAccepted) promise.m_onAccepted();
    });
    QObject::connect(dialog, &QDialog::rejected, dialog, [promise]() {
        ++s_totalRejected;
        if (promise.m_onRejected) promise.m_onRejected();
    });

    dialog->open();
    ++s_totalShows;
    return promise;
}

// ---- 构造函数 ----

/** @brief 构造AppDialog对话框实例 @param type 弹窗类型 @param title 标题文字 @param message 消息内容 @param confirmText 确认按钮文字 @param cancelText 取消按钮文字 @param parent 父控件指针 */
AppDialog::AppDialog(Type type, const QString& title, const QString& message,
                       const QString& confirmText, const QString& cancelText,
                       QWidget* parent)
    : QDialog(parent)
    , m_type(type)
    , m_confirmText(confirmText)
    , m_cancelText(cancelText)
    , m_title(title)
    , m_message(message)
{
    setObjectName("appDialog");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    setMinimumWidth(380);
    setMaximumWidth(520);

    setupUI();
    applyStyle();
}

// ---- UI构建/样式/动画方法见 AppDialogContent.cpp ----
