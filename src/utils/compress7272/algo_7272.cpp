/**
 * @file algo_7272.cpp
 */
#include "compress7272/algo_7272.h"
QVector<double> algo_7272::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
