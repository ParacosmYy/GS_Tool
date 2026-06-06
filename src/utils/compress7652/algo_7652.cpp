/**
 * @file algo_7652.cpp
 */
#include "compress7652/algo_7652.h"
QVector<double> algo_7652::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
