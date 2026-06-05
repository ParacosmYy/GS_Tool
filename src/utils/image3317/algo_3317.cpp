/**
 * @file algo_3317.cpp
 */
#include "image3317/algo_3317.h"
QVector<double> algo_3317::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
