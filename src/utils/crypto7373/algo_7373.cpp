/**
 * @file algo_7373.cpp
 */
#include "crypto7373/algo_7373.h"
QVector<double> algo_7373::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
