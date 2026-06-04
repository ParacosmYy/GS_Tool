/**
 * @file ToolbarControllerQuery.cpp
 * @brief 工具栏控制器 — 主题/语言设置与查询方法实现
 *
 * 从 ToolbarController.cpp 拆分而来，包含主题列表填充、
 * 当前主题/语言的设置与索引查询方法。
 */

#include "core/toolbar/ToolbarController.h"

#include <QComboBox>

/** @brief 设置可用主题列表(内部名转友好名，blockSignals防误触) @param themes 主题名称列表 */
void ToolbarController::setAvailableThemes(const QStringList& themes)
{
    if (!m_themeCombo) return;

    m_themeCombo->blockSignals(true);  // 填充过程中不触发信号
    m_themeCombo->clear();

    for (const QString& name : themes) {
        // 显示友好名称: dark_terminal -> Dark Terminal
        QString display = name;
        display[0] = display[0].toUpper();
        // 将下划线替换为空格并大写每个单词首字母
        QStringList parts = display.split('_');
        for (auto& part : parts) {
            if (!part.isEmpty()) part[0] = part[0].toUpper();
        }
        // itemData 存储原始名称，显示转换后的友好名称
        m_themeCombo->addItem(parts.join(" "), name);
    }

    m_themeCombo->blockSignals(false);
}

/** @brief 设置当前选中的主题(通过itemData匹配原始主题名) @param themeName 主题名称 */
void ToolbarController::setCurrentTheme(const QString& themeName)
{
    if (!m_themeCombo) return;

    for (int i = 0; i < m_themeCombo->count(); ++i) {
        if (m_themeCombo->itemData(i).toString() == themeName) {
            m_themeCombo->setCurrentIndex(i);
            break;
        }
    }
}

/** @brief 根据索引获取主题名称 @param index 下拉框索引 @return 主题原始名称，索引无效时返回空字符串 */
QString ToolbarController::themeNameAt(int index) const
{
    if (!m_themeCombo || index < 0 || index >= m_themeCombo->count()) return {};
    return m_themeCombo->itemData(index).toString();
}

/** @brief 设置当前选中的语言(通过itemData匹配语言代码) @param langCode 语言代码 */
void ToolbarController::setCurrentLanguage(const QString& langCode)
{
    if (!m_langCombo) return;

    for (int i = 0; i < m_langCombo->count(); ++i) {
        if (m_langCombo->itemData(i).toString() == langCode) {
            m_langCombo->setCurrentIndex(i);
            break;
        }
    }
}

/** @brief 根据索引获取语言代码 @param index 下拉框索引 @return 语言代码，索引无效时返回空字符串 */
QString ToolbarController::languageCodeAt(int index) const
{
    if (!m_langCombo || index < 0 || index >= m_langCombo->count()) return {};
    return m_langCombo->itemData(index).toString();
}
