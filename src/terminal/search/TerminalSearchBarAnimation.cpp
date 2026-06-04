/**
 * @file TerminalSearchBarAnimation.cpp
 * @brief 终端搜索栏 - 展开/收起动画与访问器方法实现
 *
 * 从TerminalSearchBar.cpp拆分而来，包含:
 *   - activate():   激活搜索栏并聚焦(含展开动画)
 *   - deactivate(): 关闭搜索栏(含收起动画)
 *   - onCloseClicked(): 关闭按钮点击委托
 *   - searchPattern()/isRegexMode()/isHexMode()/isCaseSensitive()/isWholeWord():
 *                   搜索参数只读访问器
 *
 * UI构建见TerminalSearchBar.cpp。
 * 搜索操作/历史管理/统计见TerminalSearchBarHistory.cpp。
 */

#include "terminal/search/TerminalSearchBar.h"
#include "shared/LayoutConstants.h"
#include "shared/AnimationConstants.h"

#include <QPropertyAnimation>

/** @brief 获取当前搜索输入框中的文本 */
QString TerminalSearchBar::searchPattern() const
{
    return m_searchInput->text();
}

/** @brief 返回正则模式复选框是否选中 */
bool TerminalSearchBar::isRegexMode() const
{
    return m_regexCheck->isChecked();
}

/** @brief 返回HEX模式复选框是否选中 */
bool TerminalSearchBar::isHexMode() const
{
    return m_hexCheck->isChecked();
}

/** @brief 返回大小写敏感复选框是否选中 */
bool TerminalSearchBar::isCaseSensitive() const
{
    return m_caseCheck->isChecked();
}

/** @brief 返回全词匹配复选框是否选中 */
bool TerminalSearchBar::isWholeWord() const
{
    return m_wordCheck->isChecked();
}

/** @brief 激活搜索栏并聚焦输入框(已可见仅聚焦，不可见播放展开动画200ms OutCubic) */
void TerminalSearchBar::activate()
{
    // 如果已经可见，仅聚焦
    if (isVisible()) {
        m_searchInput->setFocus();
        m_searchInput->selectAll();
        return;
    }

    // 展开动画: maximumHeight 从 0 到 36, 200ms, OutCubic 缓动
    setMaximumHeight(0);
    show();

    // 停止可能残留的收起动画（快速 Ctrl+F -> Esc -> Ctrl+F 场景）
    if (m_activeAnim) {
        m_activeAnim->stop();
        m_activeAnim = nullptr;
    }

    m_activeAnim = new QPropertyAnimation(this, "maximumHeight");
    m_activeAnim->setStartValue(0);
    m_activeAnim->setEndValue(36);
    m_activeAnim->setDuration(Animations::kSearchExpandMs);
    m_activeAnim->setEasingCurve(QEasingCurve::OutCubic);
    // 动画结束后恢复固定高度，避免布局异常
    connect(m_activeAnim, &QPropertyAnimation::finished, this, [this]() {
        setFixedHeight(Layout::kSearchBarHeight);
    });
    m_activeAnim->start(QAbstractAnimation::DeleteWhenStopped);

    m_searchInput->setFocus();
    m_searchInput->selectAll();
}

/** @brief 关闭搜索栏并清除内容(收起动画150ms InCubic完成后发射closed信号) */
void TerminalSearchBar::deactivate()
{
    m_searchInput->clear();
    m_resultLabel->clear();

    // 收起动画: maximumHeight 从 36 到 0, 150ms, InCubic 缓动
    if (m_activeAnim) {
        m_activeAnim->stop();
        m_activeAnim = nullptr;
    }

    m_activeAnim = new QPropertyAnimation(this, "maximumHeight");
    m_activeAnim->setStartValue(36);
    m_activeAnim->setEndValue(0);
    m_activeAnim->setDuration(Animations::kSearchCollapseMs);
    m_activeAnim->setEasingCurve(QEasingCurve::InCubic);
    connect(m_activeAnim, &QPropertyAnimation::finished, this, [this]() {
        hide();
        setFixedHeight(Layout::kSearchBarHeight);
        emit closed();
    });
    m_activeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

/** @brief 关闭按钮点击，委托给 deactivate() */
void TerminalSearchBar::onCloseClicked()
{
    deactivate();
}
