/**
 * @file algo_5637.cpp
 */
#include "image5637/algo_5637.h"
QVector<double> algo_5637::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
