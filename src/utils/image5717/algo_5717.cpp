/**
 * @file algo_5717.cpp
 */
#include "image5717/algo_5717.h"
QVector<double> algo_5717::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
