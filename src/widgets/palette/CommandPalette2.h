#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QMap>
#include <QString>
#include <functional>
#include <QVBoxLayout>

class CommandPalette : public QWidget {
    Q_OBJECT
public:
    using CommandAction = std::function<void()>;
    explicit CommandPalette(QWidget *parent = nullptr);
    ~CommandPalette() override;
    void addCommand(const QString &name, const QString &shortcut, CommandAction action);
    void removeCommand(const QString &name);
    void setFilter(const QString &text);
    void showPalette();
    void hidePalette();
    int commandCount() const;
signals:
    void commandExecuted(const QString &name);
    void paletteShown();
    void paletteHidden();
protected:
    void keyPressEvent(QKeyEvent *event) override;
private:
    void onReturnPressed();
    void onTextChanged(const QString &text);
    struct CmdEntry { QString name; QString shortcut; CommandAction action; };
    QMap<QString, CmdEntry> m_commands;
    QLineEdit *m_search = nullptr;
    QListWidget *m_list = nullptr;
};
