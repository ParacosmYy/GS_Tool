/**
 * @file algo_4853.cpp
 */
#include "crypto4853/algo_4853.h"
QVector<double> algo_4853::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
