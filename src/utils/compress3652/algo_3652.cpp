/**
 * @file algo_3652.cpp
 */
#include "compress3652/algo_3652.h"
QVector<double> algo_3652::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
