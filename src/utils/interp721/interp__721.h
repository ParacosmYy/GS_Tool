/**
 * @file interp__721.h
 * @brief interp module interp__721
 */
#pragma once
#include <QObject>
#include <QVector>
class interp__721 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit interp__721(QObject *p=nullptr) : QObject(p) {}
    ~interp__721() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

