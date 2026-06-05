/**
 * @file DataValidator.h
 * @brief 数据验证引擎 -- 用户自定义规则与模式的串行数据校验框架
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 支持6种原子规则(范围检查/正则匹配/长度检查/字节模式/校验和验证/自定义Lambda)
 * 以及3种组合逻辑(AND/OR/NOT)，可对完整帧或指定偏移字段进行验证。
 * 规则可通过JSON导入/导出，运行时统计按类型计数并追踪最常失败的规则。
 */

#ifndef DATAVALIDATOR_H
#define DATAVALIDATOR_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <functional>

/**
 * @class DataValidator
 * @brief 串行数据验证引擎
 *
 * 用户通过 addRule() 注册验证规则，之后调用 validate() 对完整帧
 * 或 validateField() 对指定偏移字段执行校验。每条规则含名称、类型
 * 和约束参数，支持 AND/OR/NOT 组合。验证结果包含 pass 标志、错误
 * 消息和失败规则索引。
 */
class DataValidator : public QObject
{
    Q_OBJECT

public:
    /** @brief 原子规则类型枚举 */
    enum class RuleType {
        RangeCheck,      ///< 数值范围检查(min/max)
        RegexMatch,      ///< 正则表达式匹配
        LengthCheck,     ///< 长度检查(min/max)
        BytePattern,     ///< 字节模式搜索(offset/pattern)
        ChecksumVerify,  ///< 校验和验证(algorithm/position)
        CustomLambda     ///< 用户自定义Lambda函数
    };
    Q_ENUM(RuleType)

    /** @brief 组合逻辑运算符 */
    enum class LogicOp {
        AND,  ///< 所有子规则都必须通过
        OR,   ///< 至少一个子规则通过即可
        NOT   ///< 子规则取反
    };
    Q_ENUM(LogicOp)

    /** @brief 预定义验证器名称 */
    enum class Predefined {
        AsciiPrintable,  ///< ASCII可打印字符(0x20~0x7E)
        Utf8Valid,       ///< UTF-8编码有效性
        HexString,       ///< 十六进制字符串([0-9A-Fa-f]+)
        BinaryData,      ///< 二进制数据(允许全部0x00~0xFF)
        NumericRange     ///< ASCII数值字符串范围(min/max)
    };
    Q_ENUM(Predefined)

    /** @brief 单条验证规则定义 */
    struct ValidationRule {
        QString name;                        ///< 规则名称(唯一标识)
        RuleType type;                       ///< 规则类型
        QVariantMap params;                  ///< 类型相关参数
        bool isComposite = false;            ///< 是否为组合规则
        LogicOp logicOp = LogicOp::AND;     ///< 组合逻辑运算符
        QStringList childRuleNames;          ///< 子规则名称列表
        QString description;                 ///< 规则描述
    };

    /** @brief 单次验证结果 */
    struct ValidationResult {
        bool pass = true;           ///< 是否通过
        QString errorMessage;       ///< 失败时的错误描述
        int failedRuleIndex = -1;   ///< 失败规则索引(-1=全部通过)
        double elapsedMs = 0.0;     ///< 验证耗时(毫秒)
    };

    /** @brief 累计统计信息 */
    struct Stats {
        quint64 totalValidations = 0;            ///< 总验证次数
        quint64 totalPasses = 0;                 ///< 通过次数
        quint64 totalFailures = 0;               ///< 失败次数
        quint64 validationsByType[6] = {};       ///< 按规则类型统计(索引=RuleType枚举值)
        double avgValidationTimeMs = 0.0;        ///< 平均验证耗时(毫秒)
        QString mostFailedRule;                  ///< 失败最多的规则名称
    };

    /** @brief 自定义验证函数类型 @param data 待验证数据 @return true=通过 */
    using CustomValidator = std::function<bool(const QByteArray &data)>;

    /** @brief 构造数据验证引擎 @param parent 父对象 */
    explicit DataValidator(QObject *parent = nullptr);
    ~DataValidator() override;  ///< 析构函数

    DataValidator(const DataValidator &) = delete;
    DataValidator &operator=(const DataValidator &) = delete;

    // ────────────── 规则管理 ──────────────

    /** @brief 添加验证规则 @param name 规则唯一名称 @param type 规则类型 @param params 类型参数 @return true=添加成功 */
    bool addRule(const QString &name, RuleType type, const QVariantMap &params);

    /** @brief 添加组合规则 @param name 规则唯一名称 @param op 逻辑运算符 @param childNames 子规则名称列表 @return true=添加成功 */
    bool addCompositeRule(const QString &name, LogicOp op, const QStringList &childNames);

    /** @brief 移除规则 @param name 规则名称 @return true=成功移除 */
    bool removeRule(const QString &name);

    /** @brief 注册自定义验证函数 @param name 规则名称(需先addRule) @param validator Lambda验证函数 @return true=注册成功 */
    bool setCustomValidator(const QString &name, CustomValidator validator);

    /** @brief 添加预定义验证规则 @param predefined 预定义类型 @param name 自定义名称(空则用默认名) @return true=添加成功 */
    bool addPredefinedRule(Predefined predefined, const QString &name = QString());

    /** @brief 获取所有规则 @return 规则列表的const引用 */
    const QList<ValidationRule> &rules() const;

    /** @brief 按名称查找规则 @param name 规则名称 @return 规则指针(nullptr=未找到) */
    const ValidationRule *findRule(const QString &name) const;

    /** @brief 清空所有规则 */
    void clearRules();

    // ────────────── 验证执行 ──────────────

    /** @brief 验证完整帧数据(按规则顺序逐一检查) @param data 帧数据 @return 验证结果 */
    ValidationResult validate(const QByteArray &data);

    /** @brief 验证指定偏移和长度的字段 @param data 帧数据 @param offset 字段偏移 @param length 字段长度 @param ruleName 使用的规则名称 @return 验证结果 */
    ValidationResult validateField(const QByteArray &data, int offset,
                                   int length, const QString &ruleName);

    // ────────────── JSON导入/导出 ──────────────

    /** @brief 导出所有规则为JSON @return JSON数组 */
    QJsonArray exportRulesToJson() const;

    /** @brief 从JSON导入规则(追加到现有规则) @param json JSON数组 @return true=全部导入成功 */
    bool importRulesFromJson(const QJsonArray &json);

    // ────────────── 统计 ──────────────

    /** @brief 获取累计统计 @return 统计结构的const引用 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 验证完成信号 @param pass 是否通过 */
    void validationComplete(bool pass);

    /** @brief 验证失败信号 @param error 错误描述 @param ruleIndex 失败规则索引 */
    void validationFailed(const QString &error, int ruleIndex);

private:
    QList<ValidationRule> m_rules;                          ///< 规则列表(按添加顺序)
    QMap<QString, CustomValidator> m_customValidators;      ///< 自定义Lambda映射
    QMap<QString, quint64> m_failureCounts;                 ///< 每条规则的失败计数
    Stats m_stats;                                          ///< 累计统计

    /** @brief 执行单条原子规则验证(按类型分发) @param rule 规则定义 @param data 待验证数据 @return true=通过 */
    bool executeAtomicRule(const ValidationRule &rule, const QByteArray &data) const;

    /** @brief 范围检查: 每个字节值在[min,max]内 @param rule 含min/max参数 @param data 数据 @return true=通过 */
    bool executeRangeCheck(const ValidationRule &rule, const QByteArray &data) const;

    /** @brief 正则匹配: UTF-8文本匹配pattern @param rule 含pattern参数 @param data 数据 @return true=匹配 */
    bool executeRegexMatch(const ValidationRule &rule, const QByteArray &data) const;

    /** @brief 长度检查: 数据长度在[min,max]内 @param rule 含min/max参数 @param data 数据 @return true=合法 */
    bool executeLengthCheck(const ValidationRule &rule, const QByteArray &data) const;

    /** @brief 字节模式搜索: 从offset查找pattern @param rule 含pattern/offset参数 @param data 数据 @return true=找到 */
    bool executeBytePattern(const ValidationRule &rule, const QByteArray &data) const;

    /** @brief 校验和验证: 计算5种算法并与期望值比较 @param rule 含algorithm/startPos/length/expected @param data 数据 @return true=匹配 */
    bool executeChecksumVerify(const ValidationRule &rule, const QByteArray &data) const;

    /** @brief 执行组合规则验证(AND/OR/NOT递归) @param rule 组合规则 @param data 待验证数据 @return true=通过 */
    bool executeCompositeRule(const ValidationRule &rule, const QByteArray &data) const;

    /** @brief 生成失败错误消息 @param rule 规则 @param index 规则索引 @return 格式化的错误字符串 */
    QString formatError(const ValidationRule &rule, int index) const;

    /** @brief 更新最常失败规则追踪 */
    void updateMostFailedRule();
};

#endif // DATAVALIDATOR_H
