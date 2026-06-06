/**
 * @file algo_7317.cpp
 */
#include "image7317/algo_7317.h"
QVector<double> algo_7317::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
