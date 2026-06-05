/**
 * @file algo_3392.cpp
 */
#include "compress3392/algo_3392.h"
QVector<double> algo_3392::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
