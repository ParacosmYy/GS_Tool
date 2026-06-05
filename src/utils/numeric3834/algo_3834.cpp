/**
 * @file algo_3834.cpp
 */
#include "numeric3834/algo_3834.h"
QVector<double> algo_3834::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
