/**
 * @file algo_3997.cpp
 */
#include "image3997/algo_3997.h"
QVector<double> algo_3997::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
