/**
 * @file algo_3659.h
 * @brief Algorithm module 3659
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_3659 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_3659(QObject *p=nullptr) : QObject(p) {}
    ~algo_3659() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
