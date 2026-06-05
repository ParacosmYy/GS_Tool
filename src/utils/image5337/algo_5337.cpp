/**
 * @file algo_5337.cpp
 */
#include "image5337/algo_5337.h"
QVector<double> algo_5337::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
