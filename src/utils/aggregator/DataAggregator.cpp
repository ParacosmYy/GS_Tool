#include "utils/aggregator/DataAggregator.h"
#include <QtMath>
#include <algorithm>

DataAggregator::DataAggregator(QObject *parent) : QObject(parent) {}
DataAggregator::~DataAggregator() = default;

void DataAggregator::addSource(const QString &name, AggregateFunc func, int window) {
    m_sources[name] = {func, window, {}, 0.0};
    emit sourceAdded(name);
}

void DataAggregator::removeSource(const QString &name) {
    m_sources.remove(name);
    emit sourceRemoved(name);
}

void DataAggregator::feedValue(const QString &source, double value) {
    auto it = m_sources.find(source);
    if (it == m_sources.end()) return;
    it->values.append(value);
    if (it->values.size() > it->windowSize) it->values.removeFirst();
    computeAggregate(source);
    emit valueAggregated(source, it->result);
}

double DataAggregator::aggregateResult(const QString &source) const {
    auto it = m_sources.constFind(source);
    return it != m_sources.constEnd() ? it->result : 0.0;
}

QMap<QString, double> DataAggregator::allResults() const {
    QMap<QString, double> results;
    for (auto it = m_sources.constBegin(); it != m_sources.constEnd(); ++it)
        results[it.key()] = it->result;
    return results;
}

QStringList DataAggregator::sources() const { return m_sources.keys(); }

void DataAggregator::setWindowSize(const QString &src, int sz) {
    auto it = m_sources.find(src);
    if (it != m_sources.end()) it->windowSize = sz;
}

void DataAggregator::setAggregateFunc(const QString &src, AggregateFunc fn) {
    auto it = m_sources.find(src);
    if (it != m_sources.end()) it->func = fn;
}

void DataAggregator::resetSource(const QString &src) {
    auto it = m_sources.find(src);
    if (it != m_sources.end()) { it->values.clear(); it->result = 0.0; }
}

void DataAggregator::resetAll() {
    for (auto it = m_sources.begin(); it != m_sources.end(); ++it) { it->values.clear(); it->result = 0.0; }
}

void DataAggregator::computeAggregate(const QString &source) {
    auto it = m_sources.find(source);
    if (it == m_sources.end() || it->values.isEmpty()) return;
    const auto &vals = it->values;
    switch (it->func) {
    case Sum: { double s=0; for (auto v:vals) s+=v; it->result=s; break; }
    case Average: { double s=0; for (auto v:vals) s+=v; it->result=s/vals.size(); break; }
    case Min: { double m=vals[0]; for (auto v:vals) if (v<m) m=v; it->result=m; break; }
    case Max: { double m=vals[0]; for (auto v:vals) if (v>m) m=v; it->result=m; break; }
    case Count: it->result=vals.size(); break;
    case First: it->result=vals.first(); break;
    case Last: it->result=vals.last(); break;
    }
}
