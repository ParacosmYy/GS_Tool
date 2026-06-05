/**
 * @file algo_3669.cpp
 */
#include "code3669/algo_3669.h"
QVector<double> algo_3669::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
