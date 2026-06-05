/**
 * @file algo_5345.cpp
 */
#include "matrix5345/algo_5345.h"
QVector<double> algo_5345::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
