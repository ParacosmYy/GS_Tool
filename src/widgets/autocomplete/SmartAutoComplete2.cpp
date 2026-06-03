#include "widgets/autocomplete/SmartAutoComplete2.h"
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QKeyEvent>
SmartAutoComplete::SmartAutoComplete(QWidget *parent) : QWidget(parent, Qt::Popup) {
    setObjectName("SmartAutoComplete2");
    auto *l = new QVBoxLayout(this); l->setContentsMargins(0,0,0,0);
    m_list = new QListWidget(this); m_list->setObjectName("autoList");
    l->addWidget(m_list);
    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) { if (m_edit) m_edit->setText(item->text()); hide(); });
}
SmartAutoComplete::~SmartAutoComplete() = default;
void SmartAutoComplete::setDictionary(const QStringList &w) { m_dictionary = w; }
void SmartAutoComplete::addWords(const QStringList &w) { m_dictionary.append(w); m_dictionary.removeDuplicates(); }
void SmartAutoComplete::setMaxSuggestions(int m) { m_maxSuggestions = m; }
void SmartAutoComplete::setCaseSensitive(bool c) { m_caseSensitive = c; }
void SmartAutoComplete::setMinCharsToTrigger(int m) { m_minChars = m; }
void SmartAutoComplete::setLineEdit(QLineEdit *e) { m_edit = e; if (e) connect(e, &QLineEdit::textChanged, this, &SmartAutoComplete::onTextChanged); }
QStringList SmartAutoComplete::suggestions() const { QStringList r; for (int i=0;i<m_list->count();i++) r<<m_list->item(i)->text(); return r; }
void SmartAutoComplete::clearDictionary() { m_dictionary.clear(); }
void SmartAutoComplete::keyPressEvent(QKeyEvent *e) { if (e->key()==Qt::Key_Escape) hide(); else QWidget::keyPressEvent(e); }
void SmartAutoComplete::onTextChanged(const QString &text) {
    if (text.length() < m_minChars) { hide(); return; }
    QStringList results;
    Qt::CaseSensitivity cs = m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    for (const auto &w : m_dictionary) { if (w.contains(text, cs)) { results << w; if (results.size() >= m_maxSuggestions) break; } }
    if (results.isEmpty()) { hide(); return; }
    showSuggestions(results);
}
void SmartAutoComplete::showSuggestions(const QStringList &items) { m_list->clear(); for (const auto &i : items) m_list->addItem(i); resize(250, qMin(items.size()*25, 200)); show(); raise(); }
