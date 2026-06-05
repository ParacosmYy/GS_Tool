/**
 * @file algo_4666.cpp
 */
#include "signal4666/algo_4666.h"
QVector<double> algo_4666::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
