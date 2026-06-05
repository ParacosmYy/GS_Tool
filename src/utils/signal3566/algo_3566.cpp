/**
 * @file algo_3566.cpp
 */
#include "signal3566/algo_3566.h"
QVector<double> algo_3566::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
