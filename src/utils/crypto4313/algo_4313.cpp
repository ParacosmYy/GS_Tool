/**
 * @file algo_4313.cpp
 */
#include "crypto4313/algo_4313.h"
QVector<double> algo_4313::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
