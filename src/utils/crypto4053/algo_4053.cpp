/**
 * @file algo_4053.cpp
 */
#include "crypto4053/algo_4053.h"
QVector<double> algo_4053::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
