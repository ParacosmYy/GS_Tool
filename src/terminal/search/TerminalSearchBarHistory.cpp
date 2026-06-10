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
#include <QRegularExpression>
#include <QStyle>
#include <QSettings>

/**
 * @brief 统一的搜索触发入口
 *
 * 根据当前搜索框内容和选项状态发射 searchRequested 或 searchCleared 信号。
 * 搜索时自动将有效模式串保存到QSettings历史(最多10条)。
 */
void TerminalSearchBar::triggerSearch()
{
    const QString text = m_searchInput->text();
    const bool regexMode = m_regexCheck->isChecked();
    const QString effectiveText = regexMode ? text : text.trimmed();
    if (effectiveText.isEmpty()) {
        m_resultLabel->clear();
        emit searchCleared();
    } else {
        ++m_totalSearches;
        if (regexMode) {
            ++m_totalRegexSearches;
        }
        if (m_hexCheck->isChecked()) {
            ++m_totalHexSearches;  ///< 统计: HEX模式搜索递增
        }
        /* 保存搜索模式到QSettings历史 */
        saveRecentSearch(effectiveText);
        emit searchRequested(effectiveText, regexMode, m_hexCheck->isChecked(),
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
    if (m_regexCheck->isChecked() && !text.isEmpty()) {
        const QRegularExpression regex(text);
        if (!regex.isValid()) {
            m_searchInput->setProperty("hasError", true);
            m_searchInput->style()->unpolish(m_searchInput);
            m_searchInput->style()->polish(m_searchInput);
            m_resultLabel->setProperty("hasError", true);
            m_resultLabel->style()->unpolish(m_resultLabel);
            m_resultLabel->style()->polish(m_resultLabel);
            m_resultLabel->setText(tr("非法正则"));
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

/** @brief 搜索选项变化时复用当前输入文本的校验路径 */
void TerminalSearchBar::refreshSearchFromControls()
{
    onSearchTextChanged(m_searchInput->text());
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

/** @brief 根据匹配总数和当前位置更新结果标签 */
void TerminalSearchBar::setMatchResult(int total, int current)
{
    if (m_searchInput->text().isEmpty()) {
        m_resultLabel->clear();
        return;
    }

    if (total <= 0) {
        m_resultLabel->setText(tr("未找到"));
        return;
    }

    m_resultLabel->setText(tr("%1/%2").arg(current + 1).arg(total));
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
    m_totalHexSearches = 0;
}

/** @brief 保存搜索模式到QSettings历史(最多10条，去重，最新在前) @param pattern 搜索模式串 */
void TerminalSearchBar::saveRecentSearch(const QString& pattern)
{
    if (pattern.trimmed().isEmpty()) return;

    /* 去重: 如果已存在则先移除旧位置 */
    m_recentSearches.removeAll(pattern);
    /* 插入到最前面 */
    m_recentSearches.prepend(pattern);
    /* 限制最多10条 */
    while (m_recentSearches.size() > 10) {
        m_recentSearches.removeLast();
    }

    /* 持久化到QSettings */
    QSettings settings;
    settings.beginGroup("TerminalSearch");
    settings.setValue("recentSearches", m_recentSearches);
    settings.endGroup();

    /* 同步更新补全器模型 */
    auto* model = qobject_cast<QStringListModel*>(m_completer->model());
    if (model) {
        model->setStringList(m_recentSearches);
    }
}

/** @brief 从QSettings加载最近搜索历史到补全器 */
void TerminalSearchBar::loadRecentSearches()
{
    QSettings settings;
    settings.beginGroup("TerminalSearch");
    m_recentSearches = settings.value("recentSearches").toStringList();
    settings.endGroup();

    /* 限制最多10条(防御性) */
    while (m_recentSearches.size() > 10) {
        m_recentSearches.removeLast();
    }

    /* 更新补全器模型 */
    auto* model = qobject_cast<QStringListModel*>(m_completer->model());
    if (model) {
        model->setStringList(m_recentSearches);
    }
}
