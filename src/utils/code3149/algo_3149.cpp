/**
 * @file algo_3149.cpp
 */
#include "code3149/algo_3149.h"
QVector<double> algo_3149::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
