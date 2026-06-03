#pragma once
#include <QWidget>
#include <QStringList>
#include <QMap>

class QLineEdit;
class QListWidget;
class SmartAutoComplete : public QWidget {
    Q_OBJECT
public:
    explicit SmartAutoComplete(QWidget *parent = nullptr);
    ~SmartAutoComplete() override;
    void setDictionary(const QStringList &words);
    void addWords(const QStringList &words);
    void setMaxSuggestions(int max);
    void setCaseSensitive(bool cs);
    void setMinCharsToTrigger(int min);
    void setLineEdit(QLineEdit *edit);
    QStringList suggestions() const;
    void clearDictionary();
protected:
    void keyPressEvent(QKeyEvent *event) override;
private:
    void onTextChanged(const QString &text);
    void showSuggestions(const QStringList &items);
    QLineEdit *m_edit = nullptr;
    QListWidget *m_list = nullptr;
    QStringList m_dictionary;
    int m_maxSuggestions = 10;
    bool m_caseSensitive = false;
    int m_minChars = 2;
};
