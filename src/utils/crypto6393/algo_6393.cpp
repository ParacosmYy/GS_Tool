/**
 * @file algo_6393.cpp
 */
#include "crypto6393/algo_6393.h"
QVector<double> algo_6393::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
