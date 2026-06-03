/**
 * @file AppDialog.cpp
 * @brief 统一弹窗组件实现 - 替代QMessageBox的自定义对话框
 *
 * 提供确认/警告/错误/信息四种弹窗类型，支持Promise链式回调，
 * 半透明背景+圆角面板+淡入动画，所有颜色通过ThemeManager获取。
 */

#include "core/widgets/AppDialog.h"
#include "core/theme/IconManager.h"
#include "core/theme/ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QShowEvent>
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

// ---- UI 构建 ----

/** @brief 初始化弹窗UI布局，包括标题行/消息内容/按钮行 */
void AppDialog::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    // 标题行: 图标 + 标题文字
    auto* titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(12);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("dialogIcon");
    m_iconLabel->setFixedSize(32, 32);
    m_iconLabel->setAlignment(Qt::AlignCenter);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("dialogTitle");
    m_titleLabel->setWordWrap(true);

    titleLayout->addWidget(m_iconLabel);
    titleLayout->addWidget(m_titleLabel, 1);
    mainLayout->addLayout(titleLayout);

    // 消息内容
    m_messageLabel = new QLabel(this);
    m_messageLabel->setObjectName("dialogMessage");
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setContentsMargins(44, 0, 0, 0); // 对齐标题文字
    mainLayout->addWidget(m_messageLabel);

    // 按钮行
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);
    buttonLayout->addStretch();

    // 取消按钮 (仅确认和警告类型显示)
    if (m_type == Type::Confirm || m_type == Type::Warning) {
        m_cancelBtn = new QPushButton(
            m_cancelText.isEmpty() ? tr("取消") : m_cancelText, this);
        m_cancelBtn->setObjectName("dialogCancelBtn");
        m_cancelBtn->setFixedHeight(36);
        m_cancelBtn->setMinimumWidth(80);
        connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
        buttonLayout->addWidget(m_cancelBtn);
    }

    // 确认按钮
    m_confirmBtn = new QPushButton(
        m_confirmText.isEmpty() ? tr("确定") : m_confirmText, this);
    m_confirmBtn->setObjectName("dialogConfirmBtn");
    m_confirmBtn->setFixedHeight(36);
    m_confirmBtn->setMinimumWidth(80);
    connect(m_confirmBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(m_confirmBtn);

    mainLayout->addLayout(buttonLayout);

    /* 设置标题和消息内容 */
    m_titleLabel->setText(m_title);
    m_messageLabel->setText(m_message);
}

/** @brief 从ThemeManager获取语义色并应用QSS样式，根据弹窗类型设置确认按钮颜色 */
void AppDialog::applyStyle()
{
    /* 从 ThemeManager 获取语义色，确保对话框跟随主题切换 */
    auto& theme = ThemeManager::instance();
    const QString bgSecondary = theme.color(ThemeManager::SemanticColor::BgSecondary).name();
    const QString borderClr = theme.color(ThemeManager::SemanticColor::Border).name();
    const QString textPrimary = theme.color(ThemeManager::SemanticColor::TextPrimary).name();
    const QString textSecondary = theme.color(ThemeManager::SemanticColor::TextSecondary).name();
    const QString bgHover = theme.color(ThemeManager::SemanticColor::BgHover).name();
    const QString accent = theme.color(ThemeManager::SemanticColor::Accent).name();
    const QString accentHover = theme.color(ThemeManager::SemanticColor::AccentHover).name();

    setStyleSheet(QString(R"(
        #appDialog {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 12px;
        }
        #dialogTitle {
            color: %3;
            font-size: 16px;
            font-weight: bold;
        }
        #dialogMessage {
            color: %4;
            font-size: 14px;
            line-height: 1.5;
        }
        #dialogCancelBtn {
            background-color: %2;
            color: %4;
            border: 1px solid %5;
            border-radius: 6px;
            padding: 0 16px;
        }
        #dialogCancelBtn:hover {
            background-color: %5;
            color: %3;
        }
        #dialogConfirmBtn {
            background-color: %6;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            padding: 0 16px;
        }
        #dialogConfirmBtn:hover {
            background-color: %7;
        }
    )").arg(bgSecondary, borderClr, textPrimary, textSecondary, bgHover, accent, accentHover));

    // 根据类型调整确认按钮颜色
    const QString warningClr = theme.color(ThemeManager::SemanticColor::Warning).name();
    const QString errorClr = theme.color(ThemeManager::SemanticColor::Error).name();

    switch (m_type) {
    case Type::Confirm:
        m_confirmBtn->setStyleSheet(
            QString("background-color: %1; color: #fff; border: none; border-radius: 6px; padding: 0 16px;"
                    "#dialogConfirmBtn:hover { background-color: %2; }").arg(accent, accentHover));
        break;
    case Type::Warning:
        m_confirmBtn->setStyleSheet(
            QString("background-color: %1; color: #fff; border: none; border-radius: 6px; padding: 0 16px;"
                    "#dialogConfirmBtn:hover { background-color: %1; }").arg(warningClr));
        break;
    case Type::Error:
        m_confirmBtn->setStyleSheet(
            QString("background-color: %1; color: #fff; border: none; border-radius: 6px; padding: 0 16px;"
                    "#dialogConfirmBtn:hover { background-color: %1; }").arg(errorClr));
        break;
    case Type::Information:
        m_confirmBtn->setStyleSheet(
            QString("background-color: %1; color: #fff; border: none; border-radius: 6px; padding: 0 16px;"
                    "#dialogConfirmBtn:hover { background-color: %2; }").arg(accent, accentHover));
        break;
    }
}

/** @brief 窗口显示事件处理，居中于父窗口并播放淡入动画 @param event 显示事件指针 */
void AppDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    // 居中显示
    if (parentWidget()) {
        auto parentRect = parentWidget()->geometry();
        move(parentRect.center() - rect().center());
    }

    // 淡入动画
    auto* effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
    effect->setOpacity(0.0);

    auto* animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(150);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}
