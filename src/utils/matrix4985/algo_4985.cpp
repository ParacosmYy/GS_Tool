/**
 * @file algo_4985.cpp
 */
#include "matrix4985/algo_4985.h"
QVector<double> algo_4985::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
