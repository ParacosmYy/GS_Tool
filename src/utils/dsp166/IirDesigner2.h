/**
 * @file IirDesigner2.h
 * @brief IIR filter design with bilinear transform
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief IIR filter design with bilinear transform
 */
class IirDesigner2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit IirDesigner2(QObject *p = nullptr) : QObject(p) {}
    ~IirDesigner2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

