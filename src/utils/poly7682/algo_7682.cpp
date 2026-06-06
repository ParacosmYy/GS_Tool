/**
 * @file algo_7682.cpp
 */
#include "poly7682/algo_7682.h"
QVector<double> algo_7682::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
