/**
 * @file algo_2981.cpp
 */
#include "interp2981/algo_2981.h"
QVector<double> algo_2981::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
