/**
 * @file algo_3801.cpp
 */
#include "interp3801/algo_3801.h"
QVector<double> algo_3801::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
