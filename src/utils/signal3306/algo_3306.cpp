/**
 * @file algo_3306.cpp
 */
#include "signal3306/algo_3306.h"
QVector<double> algo_3306::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
