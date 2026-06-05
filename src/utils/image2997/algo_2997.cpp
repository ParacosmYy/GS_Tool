/**
 * @file algo_2997.cpp
 */
#include "image2997/algo_2997.h"
QVector<double> algo_2997::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
