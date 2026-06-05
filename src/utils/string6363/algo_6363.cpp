/**
 * @file algo_6363.cpp
 */
#include "string6363/algo_6363.h"
QVector<double> algo_6363::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
