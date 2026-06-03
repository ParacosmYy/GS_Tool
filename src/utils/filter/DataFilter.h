#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QByteArray>
#include <functional>

class DataFilter : public QObject {
    Q_OBJECT
public:
    using FilterFunc = std::function<bool(const QByteArray &)>;
    explicit DataFilter(QObject *parent = nullptr);
    ~DataFilter() override;

    void addFilter(const QString &name, FilterFunc filter);
    void removeFilter(const QString &name);
    void enableFilter(const QString &name, bool enabled);
    bool process(const QByteArray &data) const;
    QByteArray apply(const QByteArray &data) const;
    QStringList activeFilters() const;
    QStringList allFilters() const;
    bool isFilterEnabled(const QString &name) const;
    void clearAll();

signals:
    void dataFiltered(const QString &filterName, bool passed);
    void filterAdded(const QString &name);
    void filterRemoved(const QString &name);

private:
    struct FilterEntry { FilterFunc func; bool enabled = true; };
    QMap<QString, FilterEntry> m_filters;
};
