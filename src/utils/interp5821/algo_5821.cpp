/**
 * @file algo_5821.cpp
 */
#include "interp5821/algo_5821.h"
QVector<double> algo_5821::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
