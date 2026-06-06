/**
 * @file algo_7757.cpp
 */
#include "image7757/algo_7757.h"
QVector<double> algo_7757::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
