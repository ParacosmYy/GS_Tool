/**
 * @file algo_4317.cpp
 */
#include "image4317/algo_4317.h"
QVector<double> algo_4317::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
