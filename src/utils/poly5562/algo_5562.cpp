/**
 * @file algo_5562.cpp
 */
#include "poly5562/algo_5562.h"
QVector<double> algo_5562::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
