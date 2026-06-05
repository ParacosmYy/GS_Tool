/**
 * @file algo_4453.cpp
 */
#include "crypto4453/algo_4453.h"
QVector<double> algo_4453::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
