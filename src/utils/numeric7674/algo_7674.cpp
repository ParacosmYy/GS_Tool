/**
 * @file algo_7674.cpp
 */
#include "numeric7674/algo_7674.h"
QVector<double> algo_7674::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
