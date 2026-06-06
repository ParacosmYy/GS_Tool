/**
 * @file algo_7493.cpp
 */
#include "crypto7493/algo_7493.h"
QVector<double> algo_7493::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
