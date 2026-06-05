/**
 * @file algo_5878.cpp
 */
#include "neural5878/algo_5878.h"
QVector<double> algo_5878::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
