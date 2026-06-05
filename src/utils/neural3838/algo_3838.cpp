/**
 * @file algo_3838.cpp
 */
#include "neural3838/algo_3838.h"
QVector<double> algo_3838::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
