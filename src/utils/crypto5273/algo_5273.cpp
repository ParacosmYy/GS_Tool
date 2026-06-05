/**
 * @file algo_5273.cpp
 */
#include "crypto5273/algo_5273.h"
QVector<double> algo_5273::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
