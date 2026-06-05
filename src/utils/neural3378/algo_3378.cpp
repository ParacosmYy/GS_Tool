/**
 * @file algo_3378.cpp
 */
#include "neural3378/algo_3378.h"
QVector<double> algo_3378::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
