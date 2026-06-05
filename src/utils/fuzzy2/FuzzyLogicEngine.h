/**
 * @file FuzzyLogicEngine.h
 * @brief 模糊逻辑引擎 — Mamdani型模糊推理
 *
 * 功能: 支持三角/梯形/高斯隶属函数，规则推理，质心去模糊化，
 *       统计推理次数/规则触发数。
 */
#ifndef FUZZYLOGICENGINE2_H
#define FUZZYLOGICENGINE2_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @class FuzzyLogicEngine
 * @brief Mamdani型模糊逻辑推理引擎
 */
class FuzzyLogicEngine : public QObject {
    Q_OBJECT
public:
    /** 隶属函数类型 */
    enum class MfType {
        Triangle,   ///< 三角形: [a, b, c]
        Trapezoid,  ///< 梯形: [a, b, c, d]
        Gaussian    ///< 高斯: [mean, sigma]
    };

    /** 模糊规则: IF input1 IS term1 AND input2 IS term2 THEN output IS outTerm */
    struct Rule {
        QMap<QString, QString> conditions; ///< inputVar → termName
        QString outputTerm;                ///< 输出模糊集名
    };

    /** 引擎统计 */
    struct Stats {
        quint64 totalEvaluations = 0;
        quint64 totalRulesFired = 0;
        double  avgRulesFired = 0.0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit FuzzyLogicEngine(QObject* parent = nullptr);

    /** 添加输入变量和隶属函数 */
    void addMembershipFunction(const QString& varName, const QString& termName,
                                MfType type, const QVector<double>& params);

    /** 设置输出变量范围 */
    void setOutputRange(double minVal, double maxVal, int resolution = 100);

    /** 添加规则 */
    void addRule(const Rule& rule);

    /** 模糊推理 */
    double evaluate(const QMap<QString, double>& inputs);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void evaluationComplete(double crispOutput, int rulesFired);

private:
    double membership(double x, MfType type, const QVector<double>& params) const;

    struct MfDef {
        MfType type;
        QVector<double> params;
    };

    QMap<QString, QMap<QString, MfDef>> m_inputMfs; ///< var → (term → mf)
    double m_outputMin;
    double m_outputMax;
    int m_outputResolution;
    QList<Rule> m_rules;
    Stats m_stats;
    double m_timeSum;
};

#endif // FUZZYLOGICENGINE2_H
