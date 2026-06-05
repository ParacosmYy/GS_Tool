/**
 * @file algo_4973.cpp
 */
#include "crypto4973/algo_4973.h"
QVector<double> algo_4973::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
