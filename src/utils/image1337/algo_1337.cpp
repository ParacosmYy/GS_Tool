/**
 * @file algo_1337.cpp
 * @brief Algorithm module 1337
 */
#include "image1337/algo_1337.h"
QVector<double> algo_1337::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
