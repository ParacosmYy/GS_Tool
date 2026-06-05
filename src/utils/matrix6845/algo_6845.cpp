/**
 * @file algo_6845.cpp
 */
#include "matrix6845/algo_6845.h"
QVector<double> algo_6845::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
