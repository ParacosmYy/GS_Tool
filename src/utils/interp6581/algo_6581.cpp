/**
 * @file algo_6581.cpp
 */
#include "interp6581/algo_6581.h"
QVector<double> algo_6581::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
