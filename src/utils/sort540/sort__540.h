/**
 * @file sort__540.h
 * @brief sort module sort__540
 */
#pragma once
#include <QObject>
#include <QVector>
class sort__540 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit sort__540(QObject *p=nullptr) : QObject(p) {}
    ~sort__540() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

