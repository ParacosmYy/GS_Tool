/**
 * @file algo_888.cpp
 * @brief Algorithm module 888
 */
#include "fft888/algo_888.h"
QVector<double> algo_888::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
