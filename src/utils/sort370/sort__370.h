/**
 * @file sort__370.h
 * @brief sort module sort__370
 */
#pragma once
#include <QObject>
#include <QVector>
class sort__370 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit sort__370(QObject *p=nullptr) : QObject(p) {}
    ~sort__370() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

