/**
 * @file algo_2938.cpp
 */
#include "neural2938/algo_2938.h"
QVector<double> algo_2938::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
