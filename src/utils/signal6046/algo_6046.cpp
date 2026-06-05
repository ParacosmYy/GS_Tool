/**
 * @file algo_6046.cpp
 */
#include "signal6046/algo_6046.h"
QVector<double> algo_6046::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
