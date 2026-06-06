/**
 * @file algo_7398.cpp
 */
#include "neural7398/algo_7398.h"
QVector<double> algo_7398::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
