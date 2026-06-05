/**
 * @file algo_6232.cpp
 */
#include "compress6232/algo_6232.h"
QVector<double> algo_6232::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
