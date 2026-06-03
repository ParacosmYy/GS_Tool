/**
 * @file DbcParser.cpp
 * @brief DBC(CAN数据库)文件解析器 - 公开接口与统计
 *
 * 支持解析Vector CANdb++格式DBC文件，包括:
 * BO_消息、SG_信号、VAL_值表、CM_注释、BA_属性、BU_节点
 *
 * 私有行解析方法见 DbcParserParse.cpp
 */

#include "protocol/can/DbcParser.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

// ──────────────────────── 构造/析构 ────────────────────────

/** @brief 构造DBC解析器 @param parent 父对象 */
DbcParser::DbcParser(QObject* parent)
    : QObject(parent)
{
}

// ──────────────────────── 公开接口 ────────────────────────

/**
 * @brief 从文件加载DBC
 * @param filePath DBC文件路径
 * @return true=解析成功
 */
bool DbcParser::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = tr("无法打开文件: %1").arg(filePath);
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString content = in.readAll();
    file.close();

    return parseFromText(content);
}

/**
 * @brief 从文本内容解析DBC
 * @param content DBC文件文本内容
 * @return true=解析成功
 */
bool DbcParser::parseFromText(const QString& content)
{
    clear();
    ++m_totalParses;

    const QStringList lines = content.split('\n');
    uint32_t currentMsgId = 0;

    for (int i = 0; i < lines.size(); ++i) {
        const QString line = lines[i].trimmed();

        /* 跳过空行和注释 */
        if (line.isEmpty() || line.startsWith(QLatin1String("//"))) {
            continue;
        }

        /* BO_: 消息定义 */
        if (line.startsWith(QLatin1String("BO_ "))) {
            if (parseMessageLine(line)) {
                static const QRegularExpression re("^BO_\\s+(\\d+)\\s+");
                const auto match = re.match(line);
                if (match.hasMatch()) {
                    currentMsgId = match.captured(1).toUInt();
                }
            }
            continue;
        }

        /* SG_: 信号定义 */
        if (line.startsWith(QLatin1String("SG_ "))) {
            parseSignalLine(line, currentMsgId);
            continue;
        }

        /* VAL_: 值表 */
        if (line.startsWith(QLatin1String("VAL_ "))) {
            parseValueTableLine(line);
            continue;
        }

        /* CM_, BA_, BU_ */
        if (line.startsWith(QLatin1String("CM_ "))) { parseCommentLine(line); continue; }
        if (line.startsWith(QLatin1String("BA_ "))) { parseAttributeLine(line); continue; }
        if (line.startsWith(QLatin1String("BU_:"))) { parseNodeLine(line); continue; }
    }

    emit parseCompleted(m_messages.size());
    return true;
}

/** @brief 获取所有消息定义 @return 消息定义列表 */
QList<DbcMessage> DbcParser::messages() const
{
    return m_messages.values();
}

/** @brief 根据消息ID查找消息定义 @param id 消息ID @return 消息定义，未找到时id=0 */
DbcMessage DbcParser::messageById(uint32_t id) const
{
    return m_messages.value(id);
}

/** @brief 根据消息名查找消息定义 @param name 消息名 @return 消息定义，未找到时id=0 */
DbcMessage DbcParser::messageByName(const QString& name) const
{
    const auto it = m_nameToId.constFind(name);
    if (it != m_nameToId.constEnd()) {
        return m_messages.value(it.value());
    }
    return DbcMessage{};
}

/**
 * @brief 解码CAN帧数据为信号值映射
 * @param msgId 消息ID
 * @param data 帧数据(最多8字节)
 * @return 信号名→物理值映射
 */
QMap<QString, double> DbcParser::decodeFrame(uint32_t msgId,
                                              const QByteArray& data) const
{
    QMap<QString, double> result;

    auto it = m_messages.constFind(msgId);
    if (it == m_messages.constEnd()) {
        return result;
    }

    const DbcMessage& msg = it.value();
    for (const DbcSignal& sig : msg.signalList) {
        const uint64_t raw = extractBits(data, sig.startBit,
                                          sig.bitLength, sig.byteOrder);
        const double physical = static_cast<double>(raw) * sig.factor + sig.offset;
        result[sig.name] = physical;
    }
    m_totalSignalsDecoded += static_cast<quint64>(msg.signalList.size());

    return result;
}

/**
 * @brief 获取信号物理值的文本描述(含值表翻译)
 * @param msgId 消息ID
 * @param signalName 信号名
 * @param rawValue 原始值
 * @return 格式化字符串
 */
QString DbcParser::formatSignalValue(uint32_t msgId,
                                      const QString& signalName,
                                      double rawValue) const
{
    auto msgIt = m_messages.constFind(msgId);
    if (msgIt == m_messages.constEnd()) { return QString(); }

    const DbcMessage& msg = msgIt.value();
    const DbcSignal* targetSig = nullptr;
    for (const DbcSignal& sig : msg.signalList) {
        if (sig.name == signalName) { targetSig = &sig; break; }
    }
    if (!targetSig) { return QString(); }

    /* 值表翻译(仅整数值) */
    const int intVal = static_cast<int>(rawValue);
    if (targetSig->valueTable.contains(intVal)) {
        return targetSig->valueTable.value(intVal);
    }

    /* 通用格式: "值 单位" */
    QString text;
    if (targetSig->factor == 1.0 && targetSig->offset == 0.0) {
        text = QString::number(intVal);
    } else {
        text = QString::number(rawValue, 'f', 2);
    }
    if (!targetSig->unit.isEmpty()) {
        text += QLatin1Char(' ') + targetSig->unit;
    }
    return text;
}

/** @brief 获取解析错误信息 @return 最近一次解析错误描述 */
QString DbcParser::lastError() const { return m_lastError; }
/** @brief 获取已解析的消息数量 @return 消息数量 */
int DbcParser::messageCount() const { return m_messages.size(); }
/** @brief 获取所有节点名称 @return 节点名称列表 */
QStringList DbcParser::nodes() const { return m_nodes; }

/** @brief 清除解析数据(消息/节点/错误信息) */
void DbcParser::clear()
{
    m_messages.clear();
    m_nameToId.clear();
    m_nodes.clear();
    m_lastError.clear();
}

// ── 统计接口 ──

/** @brief 获取累计解析次数 @return 解析次数 */
quint64 DbcParser::totalParses() const { return m_totalParses; }
/** @brief 获取累计解析的消息总数 @return 消息数 */
quint64 DbcParser::totalMessagesParsed() const { return m_totalMessagesParsed; }
/** @brief 获取累计解码的信号总数 @return 信号数 */
quint64 DbcParser::totalSignalsDecoded() const { return m_totalSignalsDecoded; }
/** @brief 获取累计解析错误次数 @return 错误次数 */
quint64 DbcParser::totalParseErrors() const { return m_totalParseErrors; }

/** @brief 重置所有DBC统计计数器(解析数/消息数/信号数/错误数) */
void DbcParser::resetDbcStatistics()
{
    m_totalParses = 0;
    m_totalMessagesParsed = 0;
    m_totalSignalsDecoded = 0;
    m_totalParseErrors = 0;
}
