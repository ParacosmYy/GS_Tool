/**
 * @file algo_3280.cpp
 */
#include "sort3280/algo_3280.h"
QVector<double> algo_3280::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
