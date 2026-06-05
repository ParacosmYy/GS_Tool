/**
 * @file algo_5569.cpp
 */
#include "code5569/algo_5569.h"
QVector<double> algo_5569::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
