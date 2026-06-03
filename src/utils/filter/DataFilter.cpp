#include "utils/filter/DataFilter.h"

DataFilter::DataFilter(QObject *parent) : QObject(parent) {}
DataFilter::~DataFilter() = default;

void DataFilter::addFilter(const QString &name, FilterFunc f) {
    m_filters[name] = {f, true};
    emit filterAdded(name);
}

void DataFilter::removeFilter(const QString &name) {
    m_filters.remove(name);
    emit filterRemoved(name);
}

void DataFilter::enableFilter(const QString &name, bool e) {
    auto it = m_filters.find(name);
    if (it != m_filters.end()) it->enabled = e;
}

bool DataFilter::process(const QByteArray &data) const {
    for (auto it = m_filters.constBegin(); it != m_filters.constEnd(); ++it) {
        if (it->enabled && !it->func(data)) return false;
    }
    return true;
}

QByteArray DataFilter::apply(const QByteArray &data) const {
    QByteArray result = data;
    for (auto it = m_filters.constBegin(); it != m_filters.constEnd(); ++it) {
        if (it->enabled) {
            bool passed = it->func(result);
            emit dataFiltered(it.key(), passed);
        }
    }
    return result;
}

QStringList DataFilter::activeFilters() const {
    QStringList l;
    for (auto it = m_filters.constBegin(); it != m_filters.constEnd(); ++it)
        if (it->enabled) l << it.key();
    return l;
}

QStringList DataFilter::allFilters() const { return m_filters.keys(); }
bool DataFilter::isFilterEnabled(const QString &n) const {
    auto it = m_filters.constFind(n);
    return it != m_filters.constEnd() && it->enabled;
}
void DataFilter::clearAll() { m_filters.clear(); }
