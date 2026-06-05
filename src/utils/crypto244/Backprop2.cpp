/**
 * @file Backprop2.cpp
 * @brief Backprop2 implementation
 */
#include "crypto244/Backprop2.h"
#include <QElapsedTimer>
QVector<double> Backprop2::compute(const QVector<double> &input) {
    QElapsedTimer t; t.start();
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

