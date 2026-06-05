/**
 * @file neural__768.h
 * @brief neural module neural__768
 */
#pragma once
#include <QObject>
#include <QVector>
class neural__768 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit neural__768(QObject *p=nullptr) : QObject(p) {}
    ~neural__768() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

