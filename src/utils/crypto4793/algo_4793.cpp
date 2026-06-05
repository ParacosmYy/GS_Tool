/**
 * @file algo_4793.cpp
 */
#include "crypto4793/algo_4793.h"
QVector<double> algo_4793::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
