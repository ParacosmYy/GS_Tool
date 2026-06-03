#pragma once
#include <QDialog>
#include <QString>
#include <QMap>

class EdDialog : public QDialog {
    Q_OBJECT
public:
    enum StandardButton { Ok, Cancel, Yes, No, Apply, Close };
    explicit EdDialog(QWidget *parent = nullptr);
    ~EdDialog() override;
    void setTitle(const QString &title);
    void setMessage(const QString &msg);
    void addButton(StandardButton btn);
    void addCustomButton(const QString &label, int roleId);
    void setIcon(const QString &iconName);
    void setContentWidget(QWidget *widget);
    int resultRole() const;
    void setRememberOption(const QString &key, const QString &label);
    bool isRememberChecked(const QString &key) const;
signals:
    void buttonClicked(int role);
private:
    void setupUi();
    struct RememberEntry { QString key; QString label; bool checked = false; };
    QMap<QString, RememberEntry> m_remember;
    int m_resultRole = 0;
};
