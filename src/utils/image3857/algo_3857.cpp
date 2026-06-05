/**
 * @file algo_3857.cpp
 */
#include "image3857/algo_3857.h"
QVector<double> algo_3857::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
