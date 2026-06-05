/**
 * @file algo_3272.cpp
 */
#include "compress3272/algo_3272.h"
QVector<double> algo_3272::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
