/**
 * @file algo_7772.cpp
 */
#include "compress7772/algo_7772.h"
QVector<double> algo_7772::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
