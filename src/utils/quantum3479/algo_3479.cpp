/**
 * @file algo_3479.cpp
 */
#include "quantum3479/algo_3479.h"
QVector<double> algo_3479::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
