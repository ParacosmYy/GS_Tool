/**
 * @file algo_7092.cpp
 */
#include "compress7092/algo_7092.h"
QVector<double> algo_7092::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
