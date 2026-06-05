/**
 * @file algo_5917.cpp
 */
#include "image5917/algo_5917.h"
QVector<double> algo_5917::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
