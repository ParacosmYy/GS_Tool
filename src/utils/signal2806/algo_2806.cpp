/**
 * @file algo_2806.cpp
 */
#include "signal2806/algo_2806.h"
QVector<double> algo_2806::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
