/**
 * @file TerminalSearchBar.h
 * @brief 终端搜索栏 - 嵌入终端顶部的搜索控件，支持文本/正则/HEX搜索
 *
 * 支持: 文本搜索、正则搜索、HEX搜索、大小写敏感、全词匹配、搜索历史
 * 快捷键: Ctrl+F 打开, Esc 关闭, F3/Shift+F3 导航匹配
 *
 * 展开/收起动画: maximumHeight 0↔36, OutCubic/InCubic缓动
 * 搜索历史: QCompleter自动补全，TerminalSearchManager维护历史列表
 * 协作: TerminalWidget(监听searchRequested/searchCleared), TerminalSearchManager(搜索历史)
 */

#ifndef TERMINALSEARCHBAR_H
#define TERMINALSEARCHBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QStringList>

/** @brief 终端搜索栏 - 嵌入终端顶部的搜索控件，支持文本/正则/HEX搜索 */
class TerminalSearchBar : public QWidget {
    Q_OBJECT
public:
    explicit TerminalSearchBar(QWidget* parent = nullptr);

    QString searchPattern() const;  ///< 获取当前搜索模式串
    bool isRegexMode() const;       ///< 是否启用正则模式
    bool isHexMode() const;         ///< 是否启用HEX模式
    bool isCaseSensitive() const;   ///< 是否启用大小写敏感
    bool isWholeWord() const;       ///< 是否启用全词匹配

public slots:
    void activate();                ///< 激活搜索栏并聚焦输入框，不可见时播放展开动画
    void deactivate();              ///< 关闭搜索栏并清除，播放收起动画
    void setResultText(const QString& text); ///< 设置匹配结果显示文本（如 "3/15"）
    void updateSearchHistory(const QStringList& history); ///< 更新搜索历史补全列表

signals:
    void searchRequested(const QString& pattern, bool regex, bool hex,  ///< 搜索触发信号
                         bool caseSensitive, bool wholeWord);
    void searchCleared(); ///< 搜索清除信号（搜索框为空时发射）
    void closed();        ///< 搜索栏关闭信号（收起动画完成后发射）

private slots:
    void onSearchTextChanged(const QString& text); ///< 搜索文本变化时触发搜索或清除
    void onCloseClicked(); ///< 关闭按钮点击，委托给deactivate()

private:
    void setupUI();               ///< 构建界面布局和样式
    bool isValidHex(const QString& text) const; ///< 验证HEX输入是否合法
    void triggerSearch();         ///< 触发当前搜索框内容的搜索请求

    QLineEdit* m_searchInput;       ///< 搜索输入框（objectName: searchBarInput）
    QPushButton* m_closeBtn;        ///< 关闭按钮（objectName: searchBarCloseBtn）
    QCheckBox* m_regexCheck;        ///< 正则模式复选框（objectName: searchBarRegexCheck）
    QCheckBox* m_hexCheck;          ///< HEX模式复选框（objectName: searchBarHexCheck）
    QCheckBox* m_caseCheck;         ///< 大小写敏感复选框（objectName: searchBarCaseCheck）
    QCheckBox* m_wordCheck;         ///< 全词匹配复选框（objectName: searchBarWordCheck）
    QLabel* m_resultLabel;          ///< 结果标签（objectName: searchBarResult），如 "3/15 matches"
    QPropertyAnimation* m_activeAnim = nullptr; ///< 当前活跃的展开/收起动画，防止快速切换时冲突
    QCompleter* m_completer;        ///< 搜索历史自动补全器

    // ---- 统计计数器 ----
    quint64 m_totalSearches = 0;        ///< 搜索触发总次数
    quint64 m_totalMatches = 0;         ///< 匹配结果总次数
    quint64 m_totalReplacements = 0;    ///< 替换操作总次数
    quint64 m_totalRegexSearches = 0;   ///< 正则搜索触发总次数
    quint64 m_totalHexSearches = 0;     ///< HEX模式搜索触发总次数

public:
    quint64 totalSearches() const { return m_totalSearches; }         ///< 搜索触发总次数
    quint64 totalMatches() const { return m_totalMatches; }          ///< 匹配结果总次数
    quint64 totalReplacements() const { return m_totalReplacements; } ///< 替换操作总次数
    quint64 totalRegexSearches() const { return m_totalRegexSearches; } ///< 正则搜索触发总次数
    quint64 totalHexSearches() const { return m_totalHexSearches; } ///< HEX模式搜索触发总次数
    void resetSearchBarStatistics(); ///< 重置搜索栏统计计数器
};

#endif // TERMINALSEARCHBAR_H
