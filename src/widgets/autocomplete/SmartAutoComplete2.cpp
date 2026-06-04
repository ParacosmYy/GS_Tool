/**
 * @file SmartAutoComplete2.cpp
 * @brief 智能自动补全v2实现 — 前缀匹配+弹出列表补全
 */
#include "widgets/autocomplete/SmartAutoComplete2.h"
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QKeyEvent>

/** @brief 构造函数，创建弹出式补全列表 @param parent 父Widget */
SmartAutoComplete::SmartAutoComplete(QWidget *parent) : QWidget(parent, Qt::Popup) {
    setObjectName("SmartAutoComplete2");
    auto *l = new QVBoxLayout(this); l->setContentsMargins(0,0,0,0);
    m_list = new QListWidget(this); m_list->setObjectName("autoList");
    l->addWidget(m_list);
    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) { if (m_edit) { ++m_totalAccepts; m_edit->setText(item->text()); } hide(); });
}
/** @brief 析构函数 */
SmartAutoComplete::~SmartAutoComplete() = default;
/** @brief 设置补全词典 @param w 词汇列表 */
void SmartAutoComplete::setDictionary(const QStringList &w) { m_dictionary = w; }
/** @brief 追加词汇到词典 @param w 词汇列表 */
void SmartAutoComplete::addWords(const QStringList &w) { m_dictionary.append(w); m_dictionary.removeDuplicates(); }
/** @brief 设置最大建议数量 @param m 最大数量 */
void SmartAutoComplete::setMaxSuggestions(int m) { m_maxSuggestions = m; }
/** @brief 设置是否区分大小写 @param c true区分大小写 */
void SmartAutoComplete::setCaseSensitive(bool c) { m_caseSensitive = c; }
/** @brief 设置触发补全的最小输入字符数 @param m 最小字符数 */
void SmartAutoComplete::setMinCharsToTrigger(int m) { m_minChars = m; }
/** @brief 绑定到QLineEdit控件，监听文本变化 @param e 目标QLineEdit */
void SmartAutoComplete::setLineEdit(QLineEdit *e) { m_edit = e; if (e) connect(e, &QLineEdit::textChanged, this, &SmartAutoComplete::onTextChanged); }
/** @brief 获取当前建议列表 @return 建议文本列表 */
QStringList SmartAutoComplete::suggestions() const { QStringList r; for (int i=0;i<m_list->count();i++) r<<m_list->item(i)->text(); return r; }
/** @brief 清空补全词典 */
void SmartAutoComplete::clearDictionary() { m_dictionary.clear(); }
/** @brief 按键事件处理 — Escape关闭弹窗 @param e 按键事件 */
void SmartAutoComplete::keyPressEvent(QKeyEvent *e) { if (e->key()==Qt::Key_Escape) hide(); else QWidget::keyPressEvent(e); }

/** @brief 文本变化时执行前缀匹配并显示建议 @param text 当前输入文本 */
void SmartAutoComplete::onTextChanged(const QString &text) {
    if (text.length() < m_minChars) { hide(); return; }
    QStringList results;
    Qt::CaseSensitivity cs = m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    for (const auto &w : m_dictionary) { if (w.contains(text, cs)) { results << w; if (results.size() >= m_maxSuggestions) break; } }
    if (results.isEmpty()) { hide(); return; }
    ++m_totalTriggers;
    showSuggestions(results);
}

/** @brief 显示建议列表 @param items 建议文本列表 */
void SmartAutoComplete::showSuggestions(const QStringList &items) { m_totalSuggestions += items.size(); m_list->clear(); for (const auto &i : items) m_list->addItem(i); resize(250, qMin(items.size()*25, 200)); show(); raise(); }
