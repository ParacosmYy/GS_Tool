/**
 * @file DataStreamFilter.cpp
 * @brief 数据流过滤器引擎核心实现
 *
 * 实现5种过滤规则类型和4种操作的链式执行引擎。
 * 规则按添加顺序依次评估，Drop立即终止链。
 */

#include "core/filter/DataStreamFilter.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSaveFile>
#include <algorithm>
#include <cmath>

// ===========================================================================
// 构造 / 规则管理
// ===========================================================================

/** @brief 构造数据流过滤器 */
DataStreamFilter::DataStreamFilter(QObject* parent)
    : QObject(parent)
    , m_nextRuleId(1)
    , m_cachedRegexRuleId(-1)
{
    memset(&m_stats, 0, sizeof(m_stats));
}

/** @brief 重新计算活跃规则数 */
static int countActiveRules(const QList<DataStreamFilter::FilterRule>& rules)
{
    return static_cast<int>(
        std::count_if(rules.cbegin(), rules.cend(),
                      [](const DataStreamFilter::FilterRule& r) { return r.enabled; }));
}

/** @brief 添加过滤规则(自动分配ID) @return 新规则ID */
int DataStreamFilter::addRule(const FilterRule& rule)
{
    FilterRule newRule = rule;
    newRule.id = m_nextRuleId++;
    m_rules.append(newRule);
    m_stats.activeRules = countActiveRules(m_rules);
    return newRule.id;
}

/** @brief 移除指定ID规则 */
bool DataStreamFilter::removeRule(int id)
{
    auto it = std::find_if(m_rules.begin(), m_rules.end(),
                           [id](const FilterRule& r) { return r.id == id; });
    if (it == m_rules.end()) {
        return false;
    }
    m_rules.erase(it);
    if (m_cachedRegexRuleId == id) {
        m_cachedRegexRuleId = -1;
        m_cachedRegex = QRegularExpression();
    }
    m_stats.activeRules = countActiveRules(m_rules);
    return true;
}

/** @brief 更新指定ID规则 */
bool DataStreamFilter::updateRule(int id, const FilterRule& rule)
{
    auto it = std::find_if(m_rules.begin(), m_rules.end(),
                           [id](const FilterRule& r) { return r.id == id; });
    if (it == m_rules.end()) {
        return false;
    }
    *it = rule;
    it->id = id;
    if (m_cachedRegexRuleId == id) {
        m_cachedRegexRuleId = -1;
        m_cachedRegex = QRegularExpression();
    }
    m_stats.activeRules = countActiveRules(m_rules);
    return true;
}

/** @brief 启用/禁用指定规则 */
void DataStreamFilter::enableRule(int id, bool enabled)
{
    auto it = std::find_if(m_rules.begin(), m_rules.end(),
                           [id](const FilterRule& r) { return r.id == id; });
    if (it != m_rules.end()) {
        it->enabled = enabled;
        m_stats.activeRules = countActiveRules(m_rules);
    }
}

/** @brief 获取所有规则列表 */
QList<DataStreamFilter::FilterRule> DataStreamFilter::rules() const
{
    return m_rules;
}

/** @brief 获取指定ID规则，未找到返回id=0默认规则 */
DataStreamFilter::FilterRule DataStreamFilter::rule(int id) const
{
    auto it = std::find_if(m_rules.cbegin(), m_rules.cend(),
                           [id](const FilterRule& r) { return r.id == id; });
    return (it != m_rules.cend()) ? *it : FilterRule{};
}

/** @brief 清空所有规则和正则缓存 */
void DataStreamFilter::clearRules()
{
    m_rules.clear();
    m_cachedRegexRuleId = -1;
    m_cachedRegex = QRegularExpression();
    m_stats.activeRules = 0;
}

// ===========================================================================
// 数据处理
// ===========================================================================

/**
 * @brief 处理单条数据
 * 依次应用所有启用规则。Drop立即终止并返回空，Modify替换数据，Alert发出信号。
 * @param input 输入数据
 * @return 处理后数据；Drop时返回空QByteArray
 */
QByteArray DataStreamFilter::process(const QByteArray& input)
{
    m_stats.totalInputPackets++;
    QByteArray current = input;

    for (const FilterRule& r : m_rules) {
        if (!r.enabled) {
            continue;
        }
        m_stats.totalRuleEvaluations++;

        if (!matchesRule(r, current)) {
            continue;
        }

        switch (r.op) {
        case FilterOp::Drop:
            m_stats.totalDropped++;
            emit packetDropped(input);
            return QByteArray();
        case FilterOp::Modify: {
            QByteArray before = current;
            current = applyRule(r, current);
            m_stats.totalModified++;
            emit packetModified(before, current);
            break;
        }
        case FilterOp::Alert:
            m_stats.totalAlerts++;
            emit alertTriggered(r.id, current);
            break;
        case FilterOp::Pass:
        default:
            break;
        }
    }

    m_stats.totalPassed++;
    emit packetPassed(current);
    return current;
}

/** @brief 批量处理数据，过滤被Drop的项 */
QList<QByteArray> DataStreamFilter::processBatch(const QList<QByteArray>& inputs)
{
    QList<QByteArray> results;
    results.reserve(inputs.size());

    if (inputs.size() > m_stats.peakQueueSize) {
        m_stats.peakQueueSize = inputs.size();
    }

    for (const QByteArray& data : inputs) {
        QByteArray processed = process(data);
        // 原始非空但处理后为空 = 被Drop
        if (!data.isEmpty() && processed.isEmpty()) {
            continue;
        }
        results.append(processed);
    }
    return results;
}

// ===========================================================================
// 规则匹配
// ===========================================================================

/** @brief 检查数据是否匹配指定规则(按FilterType分发) */
bool DataStreamFilter::matchesRule(const FilterRule& rule, const QByteArray& data)
{
    switch (rule.type) {
    case FilterType::ByteRange: {
        QByteArray target = hexToBytes(rule.pattern);
        return !target.isEmpty() && data.contains(target);
    }
    case FilterType::Regex: {
        m_stats.totalRegexMatches++;
        if (m_cachedRegexRuleId != rule.id) {
            QRegularExpression::PatternOptions options =
                QRegularExpression::UseUnicodePropertiesOption;
            if (!rule.caseSensitive) {
                options |= QRegularExpression::CaseInsensitiveOption;
            }
            m_cachedRegex = QRegularExpression(rule.pattern, options);
            m_cachedRegexRuleId = rule.id;
        }
        return m_cachedRegex.match(QString::fromUtf8(data)).hasMatch();
    }
    case FilterType::Threshold: {
        m_stats.totalThresholdChecks++;
        double value = dataToValue(data);
        return (value >= rule.minThreshold && value <= rule.maxThreshold);
    }
    case FilterType::LengthRange: {
        int len = data.size();
        if (rule.minLength > 0 && len < rule.minLength) return false;
        if (rule.maxLength > 0 && len > rule.maxLength) return false;
        return true;
    }
    case FilterType::CustomPattern: {
        QByteArray pattern = hexToBytes(rule.pattern);
        return matchHexPattern(pattern, data);
    }
    }
    return false;
}

/** @brief 应用规则操作(Modify时替换数据) */
QByteArray DataStreamFilter::applyRule(const FilterRule& rule, const QByteArray& data)
{
    if (rule.op == FilterOp::Modify && !rule.replacePattern.isEmpty()) {
        QByteArray replacement = hexToBytes(QString::fromUtf8(rule.replacePattern));
        if (!replacement.isEmpty()) {
            return replacement;
        }
    }
    return data;
}

// ===========================================================================
// 辅助方法
// ===========================================================================

/**
 * @brief Hex字符串转字节数组
 * 支持 "AA BB CC", "0xAA 0xBB", "AABBCC", "AA, BB", "??"通配符
 */
QByteArray DataStreamFilter::hexToBytes(const QString& hexStr) const
{
    QString cleaned = hexStr;
    cleaned.remove(QStringLiteral("0x"), Qt::CaseInsensitive);
    cleaned.remove(QLatin1Char(','));
    cleaned.remove(QLatin1Char(' '));
    cleaned.remove(QLatin1Char('\t'));
    cleaned.replace(QStringLiteral("??"), QStringLiteral("00"));

    if (cleaned.length() % 2 != 0) {
        return QByteArray();
    }

    QByteArray result;
    result.reserve(cleaned.length() / 2);
    bool ok = false;
    for (int i = 0; i < cleaned.length(); i += 2) {
        int byte = cleaned.mid(i, 2).toInt(&ok, 16);
        if (!ok) {
            return QByteArray();
        }
        result.append(static_cast<char>(byte));
    }
    return result;
}

/**
 * @brief 将字节数据解释为数值
 * 优先级: UTF-8文本 > float32 > double64 > int32 > int16 > 单字节
 */
double DataStreamFilter::dataToValue(const QByteArray& data) const
{
    if (data.isEmpty()) return 0.0;

    // 文本解释
    QString text = QString::fromUtf8(data).trimmed();
    bool ok = false;
    double value = text.toDouble(&ok);
    if (ok) return value;
    value = text.toInt(&ok, 16);
    if (ok) return value;

    // 二进制解释(小端序)
    if (data.size() >= 4) {
        float f;
        memcpy(&f, data.constData(), sizeof(float));
        if (std::isfinite(f) && f != 0.0f) return static_cast<double>(f);
    }
    if (data.size() >= 8) {
        double d;
        memcpy(&d, data.constData(), sizeof(double));
        if (std::isfinite(d)) return d;
    }
    if (data.size() >= 4) {
        qint32 i32;
        memcpy(&i32, data.constData(), sizeof(qint32));
        return static_cast<double>(i32);
    }
    if (data.size() >= 2) {
        qint16 i16;
        memcpy(&i16, data.constData(), sizeof(qint16));
        return static_cast<double>(i16);
    }
    return static_cast<double>(static_cast<unsigned char>(data.at(0)));
}

/** @brief 自定义Hex模式滑动窗口匹配 */
bool DataStreamFilter::matchHexPattern(const QByteArray& pattern,
                                        const QByteArray& data) const
{
    if (pattern.isEmpty() || data.isEmpty() || pattern.size() > data.size()) {
        return false;
    }

    const int patLen = pattern.size();
    const int dataLen = data.size();

    for (int i = 0; i <= dataLen - patLen; ++i) {
        bool match = true;
        for (int j = 0; j < patLen; ++j) {
            if (data.at(i + j) != pattern.at(j)) {
                match = false;
                break;
            }
        }
        if (match) return true;
    }
    return false;
}

// ===========================================================================
// 持久化 (JSON)
// ===========================================================================

/** @brief 导出规则到JSON文件 */
bool DataStreamFilter::exportRules(const QString& filePath)
{
    QJsonObject root;
    root[QStringLiteral("version")] = 1;

    QJsonArray rulesArray;
    for (const FilterRule& r : m_rules) {
        QJsonObject obj;
        obj[QStringLiteral("id")] = r.id;
        obj[QStringLiteral("type")] = static_cast<int>(r.type);
        obj[QStringLiteral("name")] = r.name;
        obj[QStringLiteral("pattern")] = r.pattern;
        obj[QStringLiteral("minThreshold")] = r.minThreshold;
        obj[QStringLiteral("maxThreshold")] = r.maxThreshold;
        obj[QStringLiteral("minLength")] = r.minLength;
        obj[QStringLiteral("maxLength")] = r.maxLength;
        obj[QStringLiteral("replacePattern")] =
            QString::fromUtf8(r.replacePattern.toHex());
        obj[QStringLiteral("op")] = static_cast<int>(r.op);
        obj[QStringLiteral("enabled")] = r.enabled;
        obj[QStringLiteral("caseSensitive")] = r.caseSensitive;
        rulesArray.append(obj);
    }
    root[QStringLiteral("rules")] = rulesArray;

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    return file.commit();
}

/** @brief 从JSON文件导入规则(清空现有后导入) */
bool DataStreamFilter::importRules(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return false;
    }

    QJsonArray rulesArray = doc.object()[QStringLiteral("rules")].toArray();
    QList<FilterRule> importedRules;

    for (const QJsonValue& val : rulesArray) {
        QJsonObject obj = val.toObject();
        FilterRule r;
        r.id = obj[QStringLiteral("id")].toInt();
        r.type = static_cast<FilterType>(obj[QStringLiteral("type")].toInt());
        r.name = obj[QStringLiteral("name")].toString();
        r.pattern = obj[QStringLiteral("pattern")].toString();
        r.minThreshold = obj[QStringLiteral("minThreshold")].toDouble();
        r.maxThreshold = obj[QStringLiteral("maxThreshold")].toDouble();
        r.minLength = obj[QStringLiteral("minLength")].toInt();
        r.maxLength = obj[QStringLiteral("maxLength")].toInt();
        r.replacePattern = QByteArray::fromHex(
            obj[QStringLiteral("replacePattern")].toString().toUtf8());
        r.op = static_cast<FilterOp>(obj[QStringLiteral("op")].toInt());
        r.enabled = obj[QStringLiteral("enabled")].toBool(true);
        r.caseSensitive = obj[QStringLiteral("caseSensitive")].toBool(false);
        importedRules.append(r);
    }

    m_rules = importedRules;
    m_cachedRegexRuleId = -1;
    m_cachedRegex = QRegularExpression();

    // 更新nextRuleId为最大ID+1
    int maxId = 0;
    for (const FilterRule& r : m_rules) {
        if (r.id > maxId) maxId = r.id;
    }
    m_nextRuleId = maxId + 1;
    m_stats.activeRules = countActiveRules(m_rules);
    return true;
}
