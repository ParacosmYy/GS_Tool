/**
 * @file algo_7789.cpp
 */
#include "code7789/algo_7789.h"
QVector<double> algo_7789::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
