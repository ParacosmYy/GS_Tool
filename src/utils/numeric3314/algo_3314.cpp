/**
 * @file algo_3314.cpp
 */
#include "numeric3314/algo_3314.h"
QVector<double> algo_3314::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
