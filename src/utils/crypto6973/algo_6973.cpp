/**
 * @file algo_6973.cpp
 */
#include "crypto6973/algo_6973.h"
QVector<double> algo_6973::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
