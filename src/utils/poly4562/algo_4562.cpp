/**
 * @file algo_4562.cpp
 */
#include "poly4562/algo_4562.h"
QVector<double> algo_4562::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
