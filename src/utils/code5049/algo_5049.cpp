/**
 * @file algo_5049.cpp
 */
#include "code5049/algo_5049.h"
QVector<double> algo_5049::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
