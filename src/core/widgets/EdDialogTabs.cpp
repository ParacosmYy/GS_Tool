/**
 * @file EdDialogTabs.cpp
 * @brief EdDialog 辅助方法实现 — 颜色/图标查询、关闭动画、静态便捷入口
 *
 * 从 EdDialog.cpp 拆分而来, 职责包括:
 * - accentColor(): 根据对话框类型返回语义强调色
 * - iconChar(): 根据对话框类型返回 Unicode 图标字符
 * - closeWithAnimation(): 反向 200ms InCubic 淡出+缩小动画后关闭
 * - confirm()/warning()/error(): 静态工厂便捷方法
 */

#include "core/widgets/EdDialog.h"

/**
 * @brief 获取当前类型的语义强调色
 *
 * 根据 DialogType 查询 ThemeManager 返回对应的语义颜色:
 * - Confirm → Accent (主题色)
 * - Warning → Warning (警告色)
 * - Error   → Error   (错误色)
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
 *
 * 返回值用于图标标签显示, 通过 QPalette 着色:
 * - Confirm → "✓"
 * - Warning → "⚠"
 * - Error   → "✕"
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
 *
 * 先将窗口缩小到 95% 并淡出, 动画结束后调用 QDialog::done()
 * 提交 m_resultCode 作为最终对话框结果。
 * 同时累加 s_totalDialogCloses 统计计数器。
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
    ++s_totalConfirmCalls;
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
    ++s_totalWarningCalls;
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
    ++s_totalErrorCalls;
    EdDialog dlg(parent, title, description, DialogType::Error);
    dlg.exec();
}

/** @brief 重置所有对话框统计计数器为零 */
void EdDialog::resetDialogStatistics()
{
    s_totalDialogOpens = 0;
    s_totalDialogCloses = 0;
    s_totalConfirmCalls = 0;
    s_totalWarningCalls = 0;
    s_totalErrorCalls = 0;
}
