/**
 * @file algo_807.cpp
 * @brief Algorithm module 807
 */
#include "dsp807/algo_807.h"
QVector<double> algo_807::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
