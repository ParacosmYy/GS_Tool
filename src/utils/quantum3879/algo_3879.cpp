/**
 * @file algo_3879.cpp
 */
#include "quantum3879/algo_3879.h"
QVector<double> algo_3879::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
