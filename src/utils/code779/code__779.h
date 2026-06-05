/**
 * @file code__779.h
 * @brief code module code__779
 */
#pragma once
#include <QObject>
#include <QVector>
class code__779 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit code__779(QObject *p=nullptr) : QObject(p) {}
    ~code__779() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

