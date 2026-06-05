/**
 * @file algo_4394.cpp
 */
#include "numeric4394/algo_4394.h"
QVector<double> algo_4394::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
