/**
 * @file DataValidator.cpp
 * @brief 数据验证引擎 -- 规则管理/验证执行/JSON导入导出/统计
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 构造函数 + 规则增删查 + validate/validateField + JSON导入导出 + 统计。
 * 原子规则执行器和组合规则执行器见 DataValidatorRules.cpp。
 */

#include "utils/validator/DataValidator.h"

#include <QRegularExpression>
#include <QStringConverter>

// ─────────────────────────── 构造/析构 ───────────────────────────

/** @brief 构造函数，初始化统计和objectName @param parent 父对象 */
DataValidator::DataValidator(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("DataValidator"));
    resetStatistics();
}

/** @brief 析构函数，清理自定义Lambda */
DataValidator::~DataValidator() = default;

// ─────────────────────────── 规则管理 ───────────────────────────

/**
 * @brief 添加验证规则
 * @param name 规则唯一名称(重复名称返回false)
 * @param type 规则类型
 * @param params 类型相关参数(RangeCheck需要min/max; RegexMatch需要pattern;
 *             LengthCheck需要min/max; BytePattern需要pattern/offset;
 *             ChecksumVerify需要algorithm/startPos/length/expected)
 * @return true=添加成功
 */
bool DataValidator::addRule(const QString &name, RuleType type,
                            const QVariantMap &params)
{
    if (name.isEmpty() || findRule(name) != nullptr)
        return false;

    ValidationRule rule;
    rule.name = name;
    rule.type = type;
    rule.params = params;
    rule.isComposite = false;
    m_rules.append(rule);
    return true;
}

/**
 * @brief 添加组合规则(AND/OR/NOT)
 * @param name 规则唯一名称
 * @param op 逻辑运算符
 * @param childNames 子规则名称列表(NOT只需一个)
 * @return true=添加成功(所有子规则必须已存在)
 */
bool DataValidator::addCompositeRule(const QString &name, LogicOp op,
                                     const QStringList &childNames)
{
    if (name.isEmpty() || findRule(name) != nullptr)
        return false;

    /* 验证所有子规则都已注册 */
    for (const QString &child : childNames) {
        if (findRule(child) == nullptr)
            return false;
    }

    ValidationRule rule;
    rule.name = name;
    rule.type = RuleType::CustomLambda; // 占位，组合规则不走原子执行
    rule.isComposite = true;
    rule.logicOp = op;
    rule.childRuleNames = childNames;
    m_rules.append(rule);
    return true;
}

/**
 * @brief 移除规则
 * @param name 规则名称
 * @return true=成功移除
 */
bool DataValidator::removeRule(const QString &name)
{
    for (int i = 0; i < m_rules.size(); ++i) {
        if (m_rules[i].name == name) {
            m_rules.removeAt(i);
            m_customValidators.remove(name);
            m_failureCounts.remove(name);
            updateMostFailedRule();
            return true;
        }
    }
    return false;
}

/**
 * @brief 注册自定义验证Lambda
 * @param name 规则名称(需先通过addRule注册CustomLambda类型规则)
 * @param validator 验证函数
 * @return true=注册成功
 */
bool DataValidator::setCustomValidator(const QString &name,
                                       CustomValidator validator)
{
    if (!validator)
        return false;

    /* 规则必须已存在且为CustomLambda类型 */
    for (auto &rule : m_rules) {
        if (rule.name == name && rule.type == RuleType::CustomLambda) {
            m_customValidators[name] = std::move(validator);
            return true;
        }
    }
    return false;
}

/**
 * @brief 添加预定义验证规则
 * @param predefined 预定义类型
 * @param name 自定义名称(空则自动生成)
 * @return true=添加成功
 */
bool DataValidator::addPredefinedRule(Predefined predefined,
                                      const QString &name)
{
    static const QMap<Predefined, QString> kDefaultNames = {
        {Predefined::AsciiPrintable, QStringLiteral("ascii_printable")},
        {Predefined::Utf8Valid,      QStringLiteral("utf8_valid")},
        {Predefined::HexString,      QStringLiteral("hex_string")},
        {Predefined::BinaryData,     QStringLiteral("binary_data")},
        {Predefined::NumericRange,   QStringLiteral("numeric_range")}
    };

    QString ruleName = name.isEmpty() ? kDefaultNames.value(predefined)
                                      : name;
    if (findRule(ruleName) != nullptr)
        return false;

    QVariantMap params;
    switch (predefined) {
    case Predefined::AsciiPrintable:
        params[QStringLiteral("min")] = 0x20;
        params[QStringLiteral("max")] = 0x7E;
        addRule(ruleName, RuleType::RangeCheck, params);
        break;
    case Predefined::Utf8Valid:
        params[QStringLiteral("encoding")] = QStringLiteral("utf8");
        addRule(ruleName, RuleType::CustomLambda, params);
        /* 注册UTF-8有效性检查Lambda */
        setCustomValidator(ruleName, [](const QByteArray &data) -> bool {
            QStringDecoder decoder(QStringDecoder::Utf8);
            if (!decoder.isValid()) return false;
            decoder.decode(data);
            return !decoder.hasError();
        });
        break;
    case Predefined::HexString:
        params[QStringLiteral("pattern")] =
            QStringLiteral("^[0-9A-Fa-f]+$");
        addRule(ruleName, RuleType::RegexMatch, params);
        break;
    case Predefined::BinaryData:
        /* 二进制数据允许所有字节，始终通过 -- 用作占位/标记 */
        params[QStringLiteral("min")] = 0;
        params[QStringLiteral("max")] = 0xFF;
        addRule(ruleName, RuleType::RangeCheck, params);
        break;
    case Predefined::NumericRange:
        params[QStringLiteral("pattern")] =
            QStringLiteral("^-?\\d+(\\.\\d+)?$");
        params[QStringLiteral("numMin")] = -1e18;
        params[QStringLiteral("numMax")] = 1e18;
        addRule(ruleName, RuleType::RegexMatch, params);
        break;
    }
    return true;
}

/** @brief 获取所有规则列表 */
const QList<DataValidator::ValidationRule> &DataValidator::rules() const
{
    return m_rules;
}

/**
 * @brief 按名称查找规则
 * @param name 规则名称
 * @return 规则指针(nullptr=未找到)
 */
const DataValidator::ValidationRule *DataValidator::findRule(
    const QString &name) const
{
    for (const auto &rule : m_rules) {
        if (rule.name == name)
            return &rule;
    }
    return nullptr;
}

/** @brief 清空所有规则、自定义Lambda和失败计数 */
void DataValidator::clearRules()
{
    m_rules.clear();
    m_customValidators.clear();
    m_failureCounts.clear();
    m_stats.mostFailedRule.clear();
}

// ─────────────────────────── 验证执行 ───────────────────────────

/**
 * @brief 验证完整帧数据，按规则顺序逐一检查，首条失败即返回
 * @param data 帧数据
 * @return 验证结果(含pass/error/index/elapsedMs)
 */
DataValidator::ValidationResult DataValidator::validate(const QByteArray &data)
{
    QElapsedTimer timer;
    timer.start();

    ValidationResult result;
    result.pass = true;
    result.failedRuleIndex = -1;

    for (int i = 0; i < m_rules.size(); ++i) {
        const ValidationRule &rule = m_rules[i];
        bool ok = rule.isComposite ? executeCompositeRule(rule, data)
                                   : executeAtomicRule(rule, data);

        /* 更新按类型统计 */
        m_stats.validationsByType[static_cast<int>(rule.type)]++;

        if (!ok) {
            result.pass = false;
            result.failedRuleIndex = i;
            result.errorMessage = formatError(rule, i);

            /* 更新失败计数 */
            m_failureCounts[rule.name]++;
            updateMostFailedRule();
            break;
        }
    }

    result.elapsedMs = static_cast<double>(timer.elapsed());

    /* 更新统计 */
    ++m_stats.totalValidations;
    if (result.pass)
        ++m_stats.totalPasses;
    else
        ++m_stats.totalFailures;

    double totalTime = m_stats.avgValidationTimeMs
                       * static_cast<double>(m_stats.totalValidations - 1);
    m_stats.avgValidationTimeMs =
        (totalTime + result.elapsedMs)
        / static_cast<double>(m_stats.totalValidations);

    emit validationComplete(result.pass);
    if (!result.pass)
        emit validationFailed(result.errorMessage, result.failedRuleIndex);

    return result;
}

/**
 * @brief 验证指定偏移和长度的字段
 * @param data 完整帧数据
 * @param offset 字段起始偏移(字节)
 * @param length 字段长度(字节)
 * @param ruleName 使用的规则名称
 * @return 验证结果
 */
DataValidator::ValidationResult DataValidator::validateField(
    const QByteArray &data, int offset, int length, const QString &ruleName)
{
    QElapsedTimer timer;
    timer.start();

    ValidationResult result;
    result.pass = true;
    result.failedRuleIndex = -1;

    /* 边界检查 */
    if (offset < 0 || length <= 0 || offset + length > data.size()) {
        result.pass = false;
        result.errorMessage =
            tr("字段偏移/长度越界: offset=%1, length=%2, dataSize=%3")
                .arg(offset).arg(length).arg(data.size());
        result.elapsedMs = static_cast<double>(timer.elapsed());
        ++m_stats.totalValidations;
        ++m_stats.totalFailures;
        emit validationComplete(false);
        emit validationFailed(result.errorMessage, -1);
        return result;
    }

    const ValidationRule *rule = findRule(ruleName);
    if (!rule) {
        result.pass = false;
        result.errorMessage = tr("规则不存在: %1").arg(ruleName);
        result.elapsedMs = static_cast<double>(timer.elapsed());
        ++m_stats.totalValidations;
        ++m_stats.totalFailures;
        emit validationComplete(false);
        emit validationFailed(result.errorMessage, -1);
        return result;
    }

    QByteArray fieldData = data.mid(offset, length);
    bool ok = rule->isComposite ? executeCompositeRule(*rule, fieldData)
                                : executeAtomicRule(*rule, fieldData);

    m_stats.validationsByType[static_cast<int>(rule->type)]++;

    if (!ok) {
        result.pass = false;
        result.failedRuleIndex = 0;
        result.errorMessage = formatError(*rule, 0);
        m_failureCounts[rule->name]++;
        updateMostFailedRule();
    }

    result.elapsedMs = static_cast<double>(timer.elapsed());

    ++m_stats.totalValidations;
    if (result.pass)
        ++m_stats.totalPasses;
    else
        ++m_stats.totalFailures;

    double totalTime = m_stats.avgValidationTimeMs
                       * static_cast<double>(m_stats.totalValidations - 1);
    m_stats.avgValidationTimeMs =
        (totalTime + result.elapsedMs)
        / static_cast<double>(m_stats.totalValidations);

    emit validationComplete(result.pass);
    if (!result.pass)
        emit validationFailed(result.errorMessage, 0);

    return result;
}

// ─────────────────────────── JSON导入/导出 ───────────────────────────

/**
 * @brief 导出所有规则为JSON数组
 * @return QJsonArray，每项为一个规则对象
 */
QJsonArray DataValidator::exportRulesToJson() const
{
    QJsonArray arr;
    for (const auto &rule : m_rules) {
        QJsonObject obj;
        obj[QStringLiteral("name")] = rule.name;
        obj[QStringLiteral("isComposite")] = rule.isComposite;

        if (rule.isComposite) {
            obj[QStringLiteral("logicOp")] =
                static_cast<int>(rule.logicOp);
            QJsonArray children;
            for (const auto &child : rule.childRuleNames)
                children.append(child);
            obj[QStringLiteral("children")] = children;
        } else {
            obj[QStringLiteral("type")] = static_cast<int>(rule.type);
            QJsonObject params;
            for (auto it = rule.params.cbegin();
                 it != rule.params.cend(); ++it) {
                params[it.key()] = QJsonValue::fromVariant(it.value());
            }
            obj[QStringLiteral("params")] = params;
        }
        obj[QStringLiteral("description")] = rule.description;
        arr.append(obj);
    }
    return arr;
}

/**
 * @brief 从JSON数组导入规则(追加到现有规则，不覆盖)
 * @param json JSON数组
 * @return true=全部导入成功
 */
bool DataValidator::importRulesFromJson(const QJsonArray &json)
{
    bool allOk = true;
    for (const QJsonValue &val : json) {
        if (!val.isObject()) { allOk = false; continue; }
        QJsonObject obj = val.toObject();

        QString name = obj[QStringLiteral("name")].toString();
        if (name.isEmpty()) { allOk = false; continue; }

        bool isComposite =
            obj[QStringLiteral("isComposite")].toBool(false);

        if (isComposite) {
            LogicOp op = static_cast<LogicOp>(
                obj[QStringLiteral("logicOp")].toInt(0));
            QStringList children;
            QJsonArray childArr =
                obj[QStringLiteral("children")].toArray();
            for (const QJsonValue &cv : childArr)
                children.append(cv.toString());
            if (!addCompositeRule(name, op, children))
                allOk = false;
        } else {
            RuleType type = static_cast<RuleType>(
                obj[QStringLiteral("type")].toInt(0));
            QVariantMap params;
            QJsonObject pObj =
                obj[QStringLiteral("params")].toObject();
            for (auto it = pObj.begin(); it != pObj.end(); ++it)
                params[it.key()] = it.value().toVariant();
            if (!addRule(name, type, params))
                allOk = false;
        }

        /* 设置描述 */
        ValidationRule *r =
            const_cast<ValidationRule *>(findRule(name));
        if (r)
            r->description =
                obj[QStringLiteral("description")].toString();
    }
    return allOk;
}

// ─────────────────────────── 统计 ───────────────────────────

/** @brief 获取累计统计 */
const DataValidator::Stats &DataValidator::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器 */
void DataValidator::resetStatistics()
{
    m_stats.totalValidations = 0;
    m_stats.totalPasses = 0;
    m_stats.totalFailures = 0;
    for (int i = 0; i < 6; ++i)
        m_stats.validationsByType[i] = 0;
    m_stats.avgValidationTimeMs = 0.0;
    m_stats.mostFailedRule.clear();
    m_failureCounts.clear();
}

// ── 原子/组合规则执行见 DataValidatorRules.cpp ──
