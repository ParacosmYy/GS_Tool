/**
 * @file TerminalSearchBarHistory.cpp
 * @brief 终端搜索栏 -- 搜索触发、历史管理、统计计数器实现
 *
 * 本文件从 TerminalSearchBar.cpp 拆分而来，专注于搜索操作与状态管理:
 *   - triggerSearch():        统一搜索触发入口，发射searchRequested/searchCleared信号
 *   - onSearchTextChanged():  搜索文本变化时验证HEX合法性并触发搜索
 *   - isValidHex():           HEX输入合法性验证(委托HexConverter)
 *   - setResultText():        设置匹配结果显示文本
 *   - updateSearchHistory():  更新搜索历史补全列表(QCompleter)
 *   - resetSearchBarStatistics(): 重置搜索栏统计计数器
 *
 * 拆分原因:
 *   TerminalSearchBar.cpp 包含UI构建、动画、搜索逻辑和历史管理，
 *   将搜索操作与历史管理独立成文件可降低单文件复杂度，便于维护。
 */

#include "terminal/search/TerminalSearchBar.h"
#include "utils/crypto/HexConverter.h"

#include <QStringListModel>
#include <QCompleter>
#include <QStyle>

/**
 * @brief 统一的搜索触发入口
 *
 * 根据当前搜索框内容和选项状态发射 searchRequested 或 searchCleared 信号。
 * 被文本变化和选项变化两种场景共用。
 */
void TerminalSearchBar::triggerSearch()
{
    const QString text = m_searchInput->text();
    if (text.isEmpty()) {
        m_resultLabel->clear();
        emit searchCleared();
    } else {
        ++m_totalSearches;
        if (m_regexCheck->isChecked()) {
            ++m_totalRegexSearches;
        }
        emit searchRequested(text, m_regexCheck->isChecked(), m_hexCheck->isChecked(),
                             m_caseCheck->isChecked(), m_wordCheck->isChecked());
    }
}

/**
 * @brief 搜索文本变化时验证HEX合法性并触发搜索
 * @param text 当前搜索框文本
 *
 * HEX模式下验证输入合法性，非法时显示错误样式。
 * 合法或非HEX模式时委托给 triggerSearch() 统一处理。
 */
void TerminalSearchBar::onSearchTextChanged(const QString& text)
{
    // HEX模式下验证输入合法性
    if (m_hexCheck->isChecked() && !text.isEmpty()) {
        if (!isValidHex(text)) {
            // 非法HEX: 通过动态属性切换错误样式
            m_searchInput->setProperty("hasError", true);
            m_searchInput->style()->unpolish(m_searchInput);
            m_searchInput->style()->polish(m_searchInput);
            m_resultLabel->setProperty("hasError", true);
            m_resultLabel->style()->unpolish(m_resultLabel);
            m_resultLabel->style()->polish(m_resultLabel);
            m_resultLabel->setText(tr("非法HEX"));
            return;
        }
    }

    // 恢复正常样式
    m_searchInput->setProperty("hasError", false);
    m_searchInput->style()->unpolish(m_searchInput);
    m_searchInput->style()->polish(m_searchInput);
    m_resultLabel->setProperty("hasError", false);
    m_resultLabel->style()->unpolish(m_resultLabel);
    m_resultLabel->style()->polish(m_resultLabel);

    triggerSearch();
}

/** @brief 验证HEX输入是否合法(委托给HexConverter::isValidHex) @param text 待验证的字符串 @return true合法 */
bool TerminalSearchBar::isValidHex(const QString& text) const
{
    return HexConverter::isValidHex(text);
}

/** @brief 设置匹配结果显示文本 @param text 要显示的结果文本(如"3/15"或"非法HEX") */
void TerminalSearchBar::setResultText(const QString& text)
{
    m_resultLabel->setText(text);
}

/**
 * @brief 更新搜索历史补全列表
 * @param history 最新的搜索历史列表
 *
 * 将历史列表设置到 QCompleter 的 QStringListModel 中，
 * 补全器会自动根据当前输入过滤匹配项。
 */
void TerminalSearchBar::updateSearchHistory(const QStringList& history)
{
    auto* model = qobject_cast<QStringListModel*>(m_completer->model());
    if (model) {
        model->setStringList(history);
    }
}

/** @brief 重置搜索栏统计计数器 */
void TerminalSearchBar::resetSearchBarStatistics()
{
    m_totalSearches = 0;
    m_totalMatches = 0;
    m_totalReplacements = 0;
    m_totalRegexSearches = 0;
}
