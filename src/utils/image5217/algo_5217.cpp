/**
 * @file algo_5217.cpp
 */
#include "image5217/algo_5217.h"
QVector<double> algo_5217::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
