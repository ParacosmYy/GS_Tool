/**
 * @file algo_4733.cpp
 */
#include "crypto4733/algo_4733.h"
QVector<double> algo_4733::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
