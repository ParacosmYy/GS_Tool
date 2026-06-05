/**
 * @file algo_3221.cpp
 */
#include "interp3221/algo_3221.h"
QVector<double> algo_3221::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
