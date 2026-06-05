/**
 * @file StateTracker.h
 * @brief 状态追踪器 — 有限状态机追踪数据流状态变迁
 *
 * 功能: 定义状态和转移规则，实时追踪数据流的状态，
 *       统计状态驻留时间/转移频率/转移路径。
 *
 * 协作: AnomalyDetector(异常状态) / EventTimeline(状态事件)
 */
#ifndef STATETRACKER_H
#define STATETRACKER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QList>
#include <QPair>
#include <QElapsedTimer>

class StateTracker : public QObject {
    Q_OBJECT
public:
    struct Transition {
        QString fromState;
        QString toState;
        qint64 timestamp;
        double triggerValue;
    };

    struct Stats {
        quint64 totalTransitions = 0;
        QMap<QString, quint64> stateVisitCounts;
        QMap<QString, double> stateDwellTime; ///< 各状态驻留时间(ms)
        int uniqueStatesVisited = 0;
    };

    explicit StateTracker(QObject* parent = nullptr);

    void addState(const QString& name);
    void addTransition(const QString& from, const QString& to, double minVal, double maxVal);
    void setInitialState(const QString& state);
    void evaluate(double value);

    QString currentState() const;
    QList<Transition> transitionHistory() const;
    void clearHistory();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void stateChanged(const QString& from, const QString& to, double value);

private:
    struct TransitionRule {
        QString toState;
        double minVal, maxVal;
    };

    QMap<QString, QList<TransitionRule>> m_transitions;
    QString m_currentState;
    QList<Transition> m_history;
    int m_maxHistory;
    QElapsedTimer m_stateTimer;
    QString m_lastState;

    Stats m_stats;
};

#endif // STATETRACKER_H
