/**
 * @file algo_4173.cpp
 */
#include "crypto4173/algo_4173.h"
QVector<double> algo_4173::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
