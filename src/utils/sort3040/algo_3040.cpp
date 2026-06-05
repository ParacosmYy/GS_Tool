/**
 * @file algo_3040.cpp
 */
#include "sort3040/algo_3040.h"
QVector<double> algo_3040::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
