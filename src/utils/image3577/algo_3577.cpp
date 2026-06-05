/**
 * @file algo_3577.cpp
 */
#include "image3577/algo_3577.h"
QVector<double> algo_3577::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
