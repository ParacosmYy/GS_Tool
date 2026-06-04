/**
 * @file EdDialog.cpp
 * @brief 自定义对话框组件实现 — 核心构造函数
 *
 * 通过 paintEvent 自绘背景/左侧边框/图标, 按钮通过 objectName 供 QSS 定位。
 * 动画: 200ms OutCubic 淡入+缩放(95%→100%), 关闭时反向。
 *
 * 绘制/动画/键盘事件见 EdDialogPaint.cpp。
 * 辅助方法(颜色/图标查询、关闭动画、静态便捷入口)见 EdDialogTabs.cpp。
 */

#include "core/widgets/EdDialog.h"

#include <QFontMetrics>

/**
 * @brief 构造对话框 — 构建 UI 布局、设置属性、连接信号
 *
 * @param parent 父窗口
 * @param title 标题文本
 * @param description 描述文本
 * @param type 对话框类型
 */
EdDialog::EdDialog(QWidget* parent, const QString& title,
                   const QString& description, DialogType type)
    : QDialog(parent), m_type(type)
{
    setObjectName("edDialog");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, false);  /* 由静态方法管理生命周期 */

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(kPadding, kPadding, kPadding, kPadding);
    mainLayout->setSpacing(12);

    /* ── 图标 + 标题行 ── */
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    auto* iconLabel = new QLabel(iconChar(), this);
    iconLabel->setObjectName("edDialogIcon");
    iconLabel->setFixedSize(kIconArea, kIconArea);
    iconLabel->setAlignment(Qt::AlignCenter);
    QFont iconFont;
    iconFont.setFamilies({"Segoe UI Emoji", "Noto Color Emoji", "Apple Color Emoji"});
    iconFont.setPointSize(kIconSize);
    iconFont.setBold(true);
    iconLabel->setFont(iconFont);

    /* 图标颜色通过 QPalette 设置, 不用硬编码 setStyleSheet */
    QPalette iconPalette = iconLabel->palette();
    iconPalette.setColor(QPalette::WindowText, accentColor());
    iconLabel->setPalette(iconPalette);
    iconLabel->setAutoFillBackground(false);

    auto* titleLabel = new QLabel(title, this);
    titleLabel->setObjectName("edDialogTitle");
    QFont titleFont;
    titleFont.setFamilies({"Microsoft YaHei UI", "Segoe UI", "Noto Sans CJK SC"});
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    QPalette titlePalette = titleLabel->palette();
    titlePalette.setColor(QPalette::WindowText,
                          ThemeManager::instance().color(ThemeManager::SemanticColor::TextPrimary));
    titleLabel->setPalette(titlePalette);

    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel, 1);

    mainLayout->addLayout(headerLayout);

    /* ── 描述文本 ── */
    auto* descLabel = new QLabel(description, this);
    descLabel->setObjectName("edDialogDescription");
    descLabel->setWordWrap(true);
    QFont descFont;
    descFont.setFamilies({"Microsoft YaHei UI", "Segoe UI", "Noto Sans CJK SC"});
    descFont.setPointSize(13);
    descLabel->setFont(descFont);
    QPalette descPalette = descLabel->palette();
    descPalette.setColor(QPalette::WindowText,
                         ThemeManager::instance().color(ThemeManager::SemanticColor::TextSecondary));
    descLabel->setPalette(descPalette);

    /* 描述区域留出左侧图标对齐的缩进 */
    descLabel->setContentsMargins(kIconArea + 10, 0, 0, 0);
    mainLayout->addWidget(descLabel);

    mainLayout->addSpacing(8);

    /* ── 按钮行 ── */
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(kButtonSpacing);
    buttonLayout->addStretch(1);

    if (m_type == DialogType::Confirm || m_type == DialogType::Warning) {
        /* 取消按钮 — 幽灵样式 */
        auto* cancelBtn = new QPushButton(tr("取消"), this);
        cancelBtn->setObjectName("edDialogCancelBtn");
        cancelBtn->setMinimumHeight(kButtonHeight);
        cancelBtn->setMinimumWidth(kButtonMinWidth);
        cancelBtn->setCursor(Qt::PointingHandCursor);
        connect(cancelBtn, &QPushButton::clicked, this, [this]() {
            m_resultCode = QDialog::Rejected;
            closeWithAnimation();
        });
        buttonLayout->addWidget(cancelBtn);
    }

    /* 确认按钮 */
    m_confirmBtn = new QPushButton(tr("确认"), this);
    m_confirmBtn->setObjectName("edDialogConfirmBtn");
    m_confirmBtn->setMinimumHeight(kButtonHeight);
    m_confirmBtn->setMinimumWidth(kButtonMinWidth);
    m_confirmBtn->setCursor(Qt::PointingHandCursor);
    connect(m_confirmBtn, &QPushButton::clicked, this, [this]() {
        m_resultCode = QDialog::Accepted;
        closeWithAnimation();
    });
    buttonLayout->addWidget(m_confirmBtn);

    mainLayout->addLayout(buttonLayout);

    /* ── 透明度特效 ── */
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(0.0);
    setGraphicsEffect(m_opacityEffect);

    /* 预计算尺寸 */
    QFontMetrics titleFm(titleFont);
    QFontMetrics descFm(descFont);
    int textWidth = 320;
    QRect descBounds = descFm.boundingRect(0, 0, textWidth - kIconArea - 10 - kPadding * 2,
                                            0, Qt::TextWordWrap, description);
    int contentHeight = kPadding * 2 + kIconArea + 12 + descBounds.height() + 8 + kButtonHeight + 16;
    setFixedSize(qMax(textWidth, 360), qMax(contentHeight, 160));
}

// 绘制/动画/键盘事件见 EdDialogPaint.cpp

