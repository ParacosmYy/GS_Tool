/**
 * @file algo_4063.cpp
 */
#include "string4063/algo_4063.h"
QVector<double> algo_4063::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
