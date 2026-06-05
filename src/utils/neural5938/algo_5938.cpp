/**
 * @file algo_5938.cpp
 */
#include "neural5938/algo_5938.h"
QVector<double> algo_5938::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
