/**
 * @file algo_2588.cpp
 * @brief Algorithm module 2588
 */
#include "fft2588/algo_2588.h"
QVector<double> algo_2588::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
