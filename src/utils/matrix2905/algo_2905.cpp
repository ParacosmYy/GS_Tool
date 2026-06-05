/**
 * @file algo_2905.cpp
 */
#include "matrix2905/algo_2905.h"
QVector<double> algo_2905::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
