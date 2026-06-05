/**
 * @file algo_5857.cpp
 */
#include "image5857/algo_5857.h"
QVector<double> algo_5857::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
