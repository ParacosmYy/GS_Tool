/**
 * @file algo_4353.cpp
 */
#include "crypto4353/algo_4353.h"
QVector<double> algo_4353::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
