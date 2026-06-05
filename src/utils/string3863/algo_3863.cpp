/**
 * @file algo_3863.cpp
 */
#include "string3863/algo_3863.h"
QVector<double> algo_3863::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
