/**
 * @file algo_4997.cpp
 */
#include "image4997/algo_4997.h"
QVector<double> algo_4997::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
