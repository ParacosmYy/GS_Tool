/**
 * @file DataValidatorRules.cpp
 * @brief 数据验证引擎 -- 原子规则执行器/组合规则执行器/辅助方法
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 从DataValidator.cpp拆分: 6种原子规则(RangeCheck/RegexMatch/LengthCheck/
 * BytePattern/ChecksumVerify/CustomLambda)执行器 + 3种组合逻辑(AND/OR/NOT)
 * 执行器 + 错误格式化 + mostFailedRule追踪。
 */

#include "utils/validator/DataValidator.h"

#include <QRegularExpression>

// ─────────────────────────── 原子规则分发 ───────────────────────────

/**
 * @brief 执行单条原子规则验证(按类型分发)
 * @param rule 规则定义
 * @param data 待验证数据
 * @return true=通过
 */
bool DataValidator::executeAtomicRule(const ValidationRule &rule,
                                      const QByteArray &data) const
{
    switch (rule.type) {
    case RuleType::RangeCheck:
        return executeRangeCheck(rule, data);
    case RuleType::RegexMatch:
        return executeRegexMatch(rule, data);
    case RuleType::LengthCheck:
        return executeLengthCheck(rule, data);
    case RuleType::BytePattern:
        return executeBytePattern(rule, data);
    case RuleType::ChecksumVerify:
        return executeChecksumVerify(rule, data);
    case RuleType::CustomLambda: {
        auto it = m_customValidators.constFind(rule.name);
        if (it != m_customValidators.cend())
            return it.value()(data);
        return true; // 未注册Lambda则默认通过
    }
    }
    return true;
}

// ─────────────────────────── 原子规则实现 ───────────────────────────

/**
 * @brief 范围检查: 每个字节值必须在[min, max]区间内
 * @param rule 含min/max参数
 * @param data 待检查数据
 * @return true=全部字节在范围内
 */
bool DataValidator::executeRangeCheck(const ValidationRule &rule,
                                      const QByteArray &data) const
{
    int minVal = rule.params.value(QStringLiteral("min")).toInt();
    int maxVal = rule.params.value(QStringLiteral("max")).toInt();
    for (int i = 0; i < data.size(); ++i) {
        int byte = static_cast<unsigned char>(data[i]);
        if (byte < minVal || byte > maxVal)
            return false;
    }
    return true;
}

/**
 * @brief 正则匹配: 将数据解码为UTF-8文本后匹配pattern
 * @param rule 含pattern参数
 * @param data 待检查数据
 * @return true=匹配成功
 */
bool DataValidator::executeRegexMatch(const ValidationRule &rule,
                                      const QByteArray &data) const
{
    QString pattern =
        rule.params.value(QStringLiteral("pattern")).toString();
    if (pattern.isEmpty())
        return false;
    QString text = QString::fromUtf8(data);
    QRegularExpression re(pattern);
    return re.match(text).hasMatch();
}

/**
 * @brief 长度检查: 数据长度在[min, max]范围内
 * @param rule 含min/max参数
 * @param data 待检查数据
 * @return true=长度合法
 */
bool DataValidator::executeLengthCheck(const ValidationRule &rule,
                                       const QByteArray &data) const
{
    int minLen = rule.params.value(QStringLiteral("min")).toInt();
    int maxLen = rule.params.value(QStringLiteral("max")).toInt();
    if (!rule.params.contains(QStringLiteral("max"))) maxLen = INT_MAX;
    int len = data.size();
    return len >= minLen && len <= maxLen;
}

/**
 * @brief 字节模式搜索: 从指定offset查找pattern
 * @param rule 含pattern和offset参数
 * @param data 待检查数据
 * @return true=找到模式
 */
bool DataValidator::executeBytePattern(const ValidationRule &rule,
                                       const QByteArray &data) const
{
    QByteArray pattern =
        rule.params.value(QStringLiteral("pattern")).toByteArray();
    int offset =
        rule.params.value(QStringLiteral("offset")).toInt(0);
    if (pattern.isEmpty())
        return false;
    if (offset < 0 || offset + pattern.size() > data.size())
        return false;
    return (data.mid(offset, pattern.size()) == pattern);
}

/**
 * @brief 校验和验证: 在指定位置计算校验和并与期望值比较
 * @param rule 含algorithm/startPos/length/expected参数
 * @param data 待检查数据
 * @return true=校验和匹配
 *
 * 支持算法: xor8, sum8, crc8, crc16_modbus, crc32
 */
bool DataValidator::executeChecksumVerify(const ValidationRule &rule,
                                          const QByteArray &data) const
{
    QString algo =
        rule.params.value(QStringLiteral("algorithm")).toString();
    int startPos =
        rule.params.value(QStringLiteral("startPos")).toInt(0);
    int len = -1;
    if (rule.params.contains(QStringLiteral("length")))
        len = rule.params.value(QStringLiteral("length")).toInt();
    quint64 expected = static_cast<quint64>(
        rule.params.value(QStringLiteral("expected")).toULongLong());

    if (startPos < 0) startPos = 0;
    if (startPos >= data.size()) return false;

    int endPos = (len < 0) ? data.size()
                           : qMin(startPos + len, data.size());
    int calcLen = endPos - startPos;
    if (calcLen <= 0) return false;

    const uint8_t *ptr =
        reinterpret_cast<const uint8_t *>(data.constData() + startPos);
    quint64 computed = 0;

    if (algo == QStringLiteral("xor8")) {
        uint8_t xorVal = 0;
        for (int i = 0; i < calcLen; ++i) xorVal ^= ptr[i];
        computed = xorVal;
    } else if (algo == QStringLiteral("sum8")) {
        uint8_t sum = 0;
        for (int i = 0; i < calcLen; ++i) sum += ptr[i];
        computed = sum;
    } else if (algo == QStringLiteral("crc8")) {
        uint8_t crc = 0x00;
        for (int i = 0; i < calcLen; ++i) {
            crc ^= ptr[i];
            for (int j = 0; j < 8; ++j)
                crc = (crc & 0x80) ? ((crc << 1) ^ 0x07) : (crc << 1);
        }
        computed = crc;
    } else if (algo == QStringLiteral("crc16_modbus")) {
        uint16_t crc = 0xFFFF;
        for (int i = 0; i < calcLen; ++i) {
            crc ^= static_cast<uint16_t>(ptr[i]);
            for (int j = 0; j < 8; ++j)
                crc = (crc & 1) ? ((crc >> 1) ^ 0xA001) : (crc >> 1);
        }
        computed = crc;
    } else if (algo == QStringLiteral("crc32")) {
        uint32_t crc = 0xFFFFFFFF;
        for (int i = 0; i < calcLen; ++i) {
            crc ^= ptr[i];
            for (int j = 0; j < 8; ++j)
                crc = (crc & 1) ? ((crc >> 1) ^ 0xEDB88320u)
                                : (crc >> 1);
        }
        computed = crc ^ 0xFFFFFFFF;
    }
    return computed == expected;
}

// ─────────────────────────── 组合规则执行 ───────────────────────────

/**
 * @brief 执行组合规则(AND/OR/NOT)，递归处理嵌套组合
 * @param rule 组合规则定义
 * @param data 待验证数据
 * @return true=组合逻辑通过
 */
bool DataValidator::executeCompositeRule(const ValidationRule &rule,
                                         const QByteArray &data) const
{
    if (rule.childRuleNames.isEmpty())
        return true;

    switch (rule.logicOp) {
    case LogicOp::AND: {
        for (const QString &childName : rule.childRuleNames) {
            const ValidationRule *child = findRule(childName);
            if (!child) continue;
            bool ok = child->isComposite
                          ? executeCompositeRule(*child, data)
                          : executeAtomicRule(*child, data);
            if (!ok) return false;
        }
        return true;
    }
    case LogicOp::OR: {
        for (const QString &childName : rule.childRuleNames) {
            const ValidationRule *child = findRule(childName);
            if (!child) continue;
            bool ok = child->isComposite
                          ? executeCompositeRule(*child, data)
                          : executeAtomicRule(*child, data);
            if (ok) return true;
        }
        return false;
    }
    case LogicOp::NOT: {
        if (rule.childRuleNames.isEmpty()) return true;
        const ValidationRule *child =
            findRule(rule.childRuleNames.first());
        if (!child) return true;
        bool ok = child->isComposite ? executeCompositeRule(*child, data)
                                     : executeAtomicRule(*child, data);
        return !ok;
    }
    }
    return true;
}

// ─────────────────────────── 辅助方法 ───────────────────────────

/**
 * @brief 生成失败错误消息(含规则类型名称和索引)
 * @param rule 规则定义
 * @param index 规则索引
 * @return 格式化的错误字符串
 */
QString DataValidator::formatError(const ValidationRule &rule,
                                   int index) const
{
    QString typeStr;
    switch (rule.type) {
    case RuleType::RangeCheck:    typeStr = tr("范围检查"); break;
    case RuleType::RegexMatch:    typeStr = tr("正则匹配"); break;
    case RuleType::LengthCheck:   typeStr = tr("长度检查"); break;
    case RuleType::BytePattern:   typeStr = tr("字节模式"); break;
    case RuleType::ChecksumVerify:typeStr = tr("校验和验证"); break;
    case RuleType::CustomLambda:  typeStr = tr("自定义规则"); break;
    }
    if (rule.isComposite) {
        QString opStr;
        switch (rule.logicOp) {
        case LogicOp::AND: opStr = QStringLiteral("AND"); break;
        case LogicOp::OR:  opStr = QStringLiteral("OR"); break;
        case LogicOp::NOT: opStr = QStringLiteral("NOT"); break;
        }
        return tr("规则#%1 [%2] 组合%3验证失败")
            .arg(index).arg(rule.name).arg(opStr);
    }
    return tr("规则#%1 [%2] %3失败")
        .arg(index).arg(rule.name).arg(typeStr);
}

/** @brief 扫描失败计数映射表，更新mostFailedRule统计字段 */
void DataValidator::updateMostFailedRule()
{
    quint64 maxCount = 0;
    QString maxName;
    for (auto it = m_failureCounts.cbegin();
         it != m_failureCounts.cend(); ++it) {
        if (it.value() > maxCount) {
            maxCount = it.value();
            maxName = it.key();
        }
    }
    m_stats.mostFailedRule = maxName;
}
