/**
 * @file algo_3472.cpp
 */
#include "compress3472/algo_3472.h"
QVector<double> algo_3472::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
