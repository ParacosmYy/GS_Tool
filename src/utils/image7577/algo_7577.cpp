/**
 * @file algo_7577.cpp
 */
#include "image7577/algo_7577.h"
QVector<double> algo_7577::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
