/**
 * @file algo_5957.cpp
 */
#include "image5957/algo_5957.h"
QVector<double> algo_5957::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
