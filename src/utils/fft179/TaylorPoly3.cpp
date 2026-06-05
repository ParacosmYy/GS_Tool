/**
 * @file TaylorPoly3.cpp
 * @brief TaylorPoly3 implementation
 */
#include "fft179/TaylorPoly3.h"
#include <QElapsedTimer>
QVector<double> TaylorPoly3::compute(const QVector<double> &input) {
    QElapsedTimer t; t.start();
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

