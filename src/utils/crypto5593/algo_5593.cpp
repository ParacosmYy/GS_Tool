/**
 * @file algo_5593.cpp
 */
#include "crypto5593/algo_5593.h"
QVector<double> algo_5593::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
