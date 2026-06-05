/**
 * @file algo_3197.cpp
 */
#include "image3197/algo_3197.h"
QVector<double> algo_3197::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
