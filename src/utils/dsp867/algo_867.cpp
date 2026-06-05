/**
 * @file algo_867.cpp
 * @brief Algorithm module 867
 */
#include "dsp867/algo_867.h"
QVector<double> algo_867::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
