/**
 * @file algo_6436.cpp
 */
#include "geometry6436/algo_6436.h"
QVector<double> algo_6436::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
