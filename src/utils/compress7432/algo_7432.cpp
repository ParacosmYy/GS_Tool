/**
 * @file algo_7432.cpp
 */
#include "compress7432/algo_7432.h"
QVector<double> algo_7432::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
