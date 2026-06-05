/**
 * @file algo_4773.cpp
 */
#include "crypto4773/algo_4773.h"
QVector<double> algo_4773::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
