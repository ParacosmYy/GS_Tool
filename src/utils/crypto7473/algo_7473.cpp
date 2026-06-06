/**
 * @file algo_7473.cpp
 */
#include "crypto7473/algo_7473.h"
QVector<double> algo_7473::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
