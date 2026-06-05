/**
 * @file algo_3281.cpp
 */
#include "interp3281/algo_3281.h"
QVector<double> algo_3281::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
