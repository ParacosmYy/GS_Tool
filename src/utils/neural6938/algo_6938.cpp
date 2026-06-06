/**
 * @file algo_6938.cpp
 */
#include "neural6938/algo_6938.h"
QVector<double> algo_6938::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
