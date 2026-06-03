#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QRect>
#include <QJsonArray>

class LayoutManager : public QObject {
    Q_OBJECT
public:
    struct LayoutItem {
        QString id;
        QString widgetType;
        QRect geometry;
        bool visible = true;
        int tabIndex = -1;
        QString parentPanel;
    };

    explicit LayoutManager(QObject *parent = nullptr);
    ~LayoutManager() override;

    void saveLayout(const QString &name);
    void loadLayout(const QString &name);
    void deleteLayout(const QString &name);
    void setCurrentItem(const QString &id, const LayoutItem &item);
    LayoutItem item(const QString &id) const;
    QStringList layouts() const;
    QStringList currentItemIds() const;
    QJsonArray serialize() const;
    void deserialize(const QJsonArray &arr);

signals:
    void layoutSaved(const QString &name);
    void layoutLoaded(const QString &name);
    void layoutDeleted(const QString &name);
    void itemChanged(const QString &id);

private:
    QMap<QString, LayoutItem> m_items;
    QMap<QString, QMap<QString, LayoutItem>> m_savedLayouts;
    QString m_currentLayout;
};
