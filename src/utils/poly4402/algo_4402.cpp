/**
 * @file algo_4402.cpp
 */
#include "poly4402/algo_4402.h"
QVector<double> algo_4402::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
