/**
 * @file EdDialog.cpp
 * @brief 自定义对话框组件实现 — 统一替代 QMessageBox 的语义化弹窗
 *
 * 通过 paintEvent 自绘背景/左侧边框/图标, 按钮通过 objectName 供 QSS 定位。
 * 动画: 200ms OutCubic 淡入+缩放(95%→100%), 关闭时反向。
 */

#include "core/widgets/EdDialog.h"

#include <QKeyEvent>
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

/**
 * @brief 自绘事件 — 绘制圆角背景 + 左侧强调色边框
 */
void EdDialog::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    auto& theme = ThemeManager::instance();
    QColor bg = theme.color(ThemeManager::SemanticColor::BgSecondary);
    QColor accent = accentColor();

    /* 圆角背景 */
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(1, 1, -1, -1), kRadius, kRadius);
    p.fillPath(path, bg);

    /* 左侧强调色边框 */
    p.save();
    p.setClipPath(path);
    p.fillRect(QRect(0, 0, kLeftBorder, height()), accent);
    p.restore();
}

/**
 * @brief 显示事件 — 触发 200ms OutCubic 淡入+缩放动画
 */
void EdDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    ++s_totalDialogOpens;

    /* 缩放动画: 95% → 100% */
    auto* scaleAnim = new QPropertyAnimation(this, "geometry");
    QRect finalGeo = geometry();
    QPoint center = finalGeo.center();
    QRect startGeo = finalGeo;
    startGeo.setWidth(static_cast<int>(finalGeo.width() * 0.95));
    startGeo.setHeight(static_cast<int>(finalGeo.height() * 0.95));
    startGeo.moveCenter(center);
    setGeometry(startGeo);
    scaleAnim->setStartValue(startGeo);
    scaleAnim->setEndValue(finalGeo);
    scaleAnim->setDuration(200);
    scaleAnim->setEasingCurve(QEasingCurve::OutCubic);

    /* 淡入动画: 0.0 → 1.0 */
    auto* fadeAnim = new QPropertyAnimation(m_opacityEffect, "opacity");
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setDuration(200);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);

    auto* group = new QParallelAnimationGroup(this);
    group->addAnimation(scaleAnim);
    group->addAnimation(fadeAnim);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

/**
 * @brief 键盘事件 — Escape 键关闭对话框
 */
void EdDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        m_resultCode = QDialog::Rejected;
        closeWithAnimation();
        return;
    }
    QDialog::keyPressEvent(event);
}

/**
 * @brief 获取当前类型的语义强调色
 */
QColor EdDialog::accentColor() const
{
    using SC = ThemeManager::SemanticColor;
    switch (m_type) {
    case DialogType::Confirm:  return ThemeManager::instance().color(SC::Accent);
    case DialogType::Warning:  return ThemeManager::instance().color(SC::Warning);
    case DialogType::Error:    return ThemeManager::instance().color(SC::Error);
    }
    return ThemeManager::instance().color(SC::Accent);
}

/**
 * @brief 获取当前类型的图标 Unicode 字符
 */
QString EdDialog::iconChar() const
{
    switch (m_type) {
    case DialogType::Confirm:  return tr("✓");
    case DialogType::Warning:  return tr("⚠");
    case DialogType::Error:    return tr("✕");
    }
    return tr("✓");
}

/**
 * @brief 关闭动画 — 反向 200ms InCubic 淡出+缩小后关闭
 */
void EdDialog::closeWithAnimation()
{
    QRect currentGeo = geometry();
    QPoint center = currentGeo.center();
    QRect endGeo = currentGeo;
    endGeo.setWidth(static_cast<int>(currentGeo.width() * 0.95));
    endGeo.setHeight(static_cast<int>(currentGeo.height() * 0.95));
    endGeo.moveCenter(center);

    auto* scaleAnim = new QPropertyAnimation(this, "geometry");
    scaleAnim->setStartValue(currentGeo);
    scaleAnim->setEndValue(endGeo);
    scaleAnim->setDuration(200);
    scaleAnim->setEasingCurve(QEasingCurve::InCubic);

    auto* fadeAnim = new QPropertyAnimation(m_opacityEffect, "opacity");
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);
    fadeAnim->setDuration(200);
    fadeAnim->setEasingCurve(QEasingCurve::InCubic);

    auto* group = new QParallelAnimationGroup(this);
    group->addAnimation(scaleAnim);
    group->addAnimation(fadeAnim);
    connect(group, &QAbstractAnimation::finished, this, [this]() {
        ++s_totalDialogCloses;
        QDialog::done(m_resultCode);
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

/**
 * @brief 静态便捷方法 — 确认对话框
 *
 * 显示确认/取消双按钮对话框, 返回用户是否点击了确认。
 *
 * @param parent 父窗口
 * @param title 标题文本
 * @param description 描述文本
 * @return true 用户点击确认, false 用户点击取消或关闭
 */
bool EdDialog::confirm(QWidget* parent, const QString& title,
                       const QString& description)
{
    EdDialog dlg(parent, title, description, DialogType::Confirm);
    int result = dlg.exec();
    return result == QDialog::Accepted;
}

/**
 * @brief 静态便捷方法 — 警告对话框
 *
 * 显示带警告图标和左侧 Warning 色边框的对话框。
 *
 * @param parent 父窗口
 * @param title 标题文本
 * @param description 描述文本
 */
void EdDialog::warning(QWidget* parent, const QString& title,
                       const QString& description)
{
    EdDialog dlg(parent, title, description, DialogType::Warning);
    dlg.exec();
}

/**
 * @brief 静态便捷方法 — 错误对话框
 *
 * 显示带错误图标和左侧 Error 色边框的单按钮对话框。
 *
 * @param parent 父窗口
 * @param title 标题文本
 * @param description 描述文本
 */
void EdDialog::error(QWidget* parent, const QString& title,
                     const QString& description)
{
    EdDialog dlg(parent, title, description, DialogType::Error);
    dlg.exec();
}
