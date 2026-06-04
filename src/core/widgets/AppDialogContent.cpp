/**
 * @file AppDialogContent.cpp
 * @brief AppDialog 内容构建器与动画方法实现
 *
 * 从 AppDialog.cpp 拆分而来，包含 UI 布局构建(setupUI)、
 * 主题样式应用(applyStyle)和窗口显示动画(showEvent)三个核心方法。
 * 所有颜色通过 ThemeManager 语义色获取，不硬编码。
 */

#include "core/widgets/AppDialog.h"
#include "core/theme/ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QShowEvent>

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
