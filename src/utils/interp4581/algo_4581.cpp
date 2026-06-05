/**
 * @file algo_4581.cpp
 */
#include "interp4581/algo_4581.h"
QVector<double> algo_4581::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
