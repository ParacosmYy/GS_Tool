/**
 * @file algo_4593.cpp
 */
#include "crypto4593/algo_4593.h"
QVector<double> algo_4593::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
