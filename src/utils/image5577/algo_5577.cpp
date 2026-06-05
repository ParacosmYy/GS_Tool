/**
 * @file algo_5577.cpp
 */
#include "image5577/algo_5577.h"
QVector<double> algo_5577::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
