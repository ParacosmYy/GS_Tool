/**
 * @file FuzzyLogicEngine.h
 * @brief 模糊逻辑引擎 — Mamdani推理与重心法去模糊化
 *
 * 支持三角形/梯形隶属函数、Mamdani规则推理和重心法去模糊化,
 * 适用于嵌入式调试场景中的阈值自适应、信号分类和决策支持。
 */
#ifndef FUZZYLOGICENGINE2_H
#define FUZZYLOGICENGINE2_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QStringList>

/**
 * @class FuzzyLogicEngine
 * @brief 模糊逻辑推理引擎
 *
 * 典型用法:
 * @code
 *   FuzzyLogicEngine engine;
 *   engine.addMembershipFunction("temp", "cold", {0, 0, 20, 30});
 *   engine.addMembershipFunction("temp", "hot",  {20, 30, 50, 50});
 *   engine.addRule({"cold"}, "fan", "low");
 *   double result = engine.evaluate({{"temp", 25.0}});
 * @endcode
 */
class FuzzyLogicEngine : public QObject {
    Q_OBJECT

public:
    /** @brief 隶属函数形状: 梯形参数 {a, b, c, d} (三角形时 b==c) */
    struct TrapezoidMF {
        double a = 0.0;  ///< 左底边界
        double b = 0.0;  ///< 左顶边界
        double c = 0.0;  ///< 右顶边界
        double d = 0.0;  ///< 右底边界
    };

    /** @brief 模糊规则: IF inputs满足 THEN output为term */
    struct FuzzyRule {
        QStringList inputTerms;  ///< 输入变量对应的模糊项名
        QString outputVariable;  ///< 输出变量名
        QString outputTerm;      ///< 输出模糊项名
    };

    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalEvaluations = 0;      ///< 推理总次数
        quint64 totalRulesFired = 0;       ///< 规则触发总次数
        double  avgProcessingTime = 0.0;   ///< 平均处理时间(us)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit FuzzyLogicEngine(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~FuzzyLogicEngine() override;

    // ── 配置 ──

    /**
     * @brief 添加隶属函数(梯形/三角形)
     * @param variable 变量名
     * @param term     模糊项名(如 "cold")
     * @param mf       梯形参数 {a,b,c,d}
     */
    void addMembershipFunction(const QString& variable,
                               const QString& term,
                               const TrapezoidMF& mf);

    /**
     * @brief 添加三角形隶属函数(便捷接口)
     * @param variable 变量名
     * @param term     模糊项名
     * @param a 左底
     * @param b 顶点
     * @param c 右底
     */
    void addTriangleMF(const QString& variable, const QString& term,
                       double a, double b, double c);

    /**
     * @brief 添加模糊规则
     * @param inputTerms    输入项名列表(按变量注册顺序)
     * @param outputVar     输出变量名
     * @param outputTerm    输出模糊项名
     */
    void addRule(const QStringList& inputTerms,
                 const QString& outputVar, const QString& outputTerm);

    /**
     * @brief 设置去模糊化分辨率
     * @param resolution 积分离散点数, 默认200
     */
    void setDefuzzResolution(int resolution);

    // ── 推理 ──

    /**
     * @brief 执行模糊推理并去模糊化
     * @param inputs 变量名→精确值映射
     * @return 去模糊化后的精确输出值; 无规则触发返回0.0
     */
    double evaluate(const QMap<QString, double>& inputs);

    // ── 辅助 ──

    /**
     * @brief 计算指定隶属函数值
     * @param variable 变量名
     * @param term     模糊项名
     * @param x        精确值
     * @return 隶属度 [0.0, 1.0]
     */
    double membershipDegree(const QString& variable,
                            const QString& term, double x) const;

    /** @brief 清除所有规则和隶属函数 */
    void clear();

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 推理完成信号 @param result 去模糊化结果 @param rulesFired 触发规则数 */
    void evaluated(double result, int rulesFired);
    /** @brief 错误信号 @param errorMessage 错误描述 */
    void error(const QString& errorMessage);

private:
    /** @brief 计算梯形隶属度 @param mf 梯形参数 @param x 输入值 */
    double trapezoidValue(const TrapezoidMF& mf, double x) const;

    /** @brief 重心法去模糊化 @param clippedHeights 每个输出项的裁剪高度 */
    double centroidDefuzzify(
        const QMap<QString, double>& clippedHeights) const;

    /** @brief 更新平均处理时间(us) */
    void updateAvgTime(qint64 elapsedUs);

    /** @brief 变量→{项名→隶属函数} */
    QMap<QString, QMap<QString, TrapezoidMF>> m_membershipFunctions;

    /** @brief 输入变量注册顺序(与规则中 inputTerms 顺序对应) */
    QStringList m_inputVariables;

    /** @brief 模糊规则列表 */
    QVector<FuzzyRule> m_rules;

    /** @brief 去模糊化分辨率 */
    int m_defuzzResolution = 200;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // FUZZYLOGICENGINE2_H
