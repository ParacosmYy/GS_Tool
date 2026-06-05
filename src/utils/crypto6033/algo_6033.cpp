/**
 * @file algo_6033.cpp
 */
#include "crypto6033/algo_6033.h"
QVector<double> algo_6033::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
