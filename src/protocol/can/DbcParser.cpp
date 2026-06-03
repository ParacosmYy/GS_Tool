/**
 * @file DbcParser.cpp
 * @brief DBC(CAN数据库)文件解析器实现
 *
 * 支持解析Vector CANdb++格式DBC文件，包括:
 * BO_消息、SG_信号、VAL_值表、CM_注释、BA_属性、BU_节点
 */

#include "protocol/can/DbcParser.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

// ──────────────────────── 构造/析构 ────────────────────────

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
                /* 解析成功后，当前消息ID更新 */
                static const QRegularExpression re(
                    "^BO_\\s+(\\d+)\\s+");
                const auto match = re.match(line);
                if (match.hasMatch()) {
                    currentMsgId = match.captured(1).toUInt();
                }
            }
            continue;
        }

        /* SG_: 信号定义(可能在BO_后面缩进出现，也可能不缩进) */
        if (line.startsWith(QLatin1String("SG_ "))) {
            parseSignalLine(line, currentMsgId);
            continue;
        }

        /* VAL_: 值表 */
        if (line.startsWith(QLatin1String("VAL_ "))) {
            parseValueTableLine(line);
            continue;
        }

        /* CM_: 注释 */
        if (line.startsWith(QLatin1String("CM_ "))) {
            parseCommentLine(line);
            continue;
        }

        /* BA_: 属性定义 */
        if (line.startsWith(QLatin1String("BA_ "))) {
            parseAttributeLine(line);
            continue;
        }

        /* BU_: 节点定义 */
        if (line.startsWith(QLatin1String("BU_:"))) {
            parseNodeLine(line);
            continue;
        }
    }

    emit parseCompleted(m_messages.size());
    return true;
}

/**
 * @brief 获取所有消息定义
 */
QList<DbcMessage> DbcParser::messages() const
{
    return m_messages.values();
}

/**
 * @brief 根据消息ID查找消息定义
 * @param id 消息ID
 * @return 消息定义，未找到时id=0
 */
DbcMessage DbcParser::messageById(uint32_t id) const
{
    return m_messages.value(id);
}

/**
 * @brief 根据消息名查找消息定义
 * @param name 消息名
 * @return 消息定义，未找到时id=0
 */
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
 * @return 格式化字符串(如 "45.0 km/h" 或 "Active")
 */
QString DbcParser::formatSignalValue(uint32_t msgId,
                                      const QString& signalName,
                                      double rawValue) const
{
    auto msgIt = m_messages.constFind(msgId);
    if (msgIt == m_messages.constEnd()) {
        return QString();
    }

    /* 查找信号定义 */
    const DbcMessage& msg = msgIt.value();
    const DbcSignal* targetSig = nullptr;
    for (const DbcSignal& sig : msg.signalList) {
        if (sig.name == signalName) {
            targetSig = &sig;
            break;
        }
    }

    if (!targetSig) {
        return QString();
    }

    /* 检查值表翻译(仅整数值) */
    const int intVal = static_cast<int>(rawValue);
    if (targetSig->valueTable.contains(intVal)) {
        return targetSig->valueTable.value(intVal);
    }

    /* 通用格式: "值 单位" */
    QString text;
    if (targetSig->factor == 1.0 && targetSig->offset == 0.0) {
        /* 整型信号 */
        text = QString::number(intVal);
    } else {
        text = QString::number(rawValue, 'f', 2);
    }

    if (!targetSig->unit.isEmpty()) {
        text += QLatin1Char(' ') + targetSig->unit;
    }

    return text;
}

/**
 * @brief 获取解析错误信息
 */
QString DbcParser::lastError() const
{
    return m_lastError;
}

/**
 * @brief 获取已解析的消息数量
 */
int DbcParser::messageCount() const
{
    return m_messages.size();
}

/**
 * @brief 获取所有节点名称
 */
QStringList DbcParser::nodes() const
{
    return m_nodes;
}

/**
 * @brief 清除解析数据
 */
void DbcParser::clear()
{
    m_messages.clear();
    m_nameToId.clear();
    m_nodes.clear();
    m_lastError.clear();
}

// ──────────────────────── 私有: 行解析器 ────────────────────────

/**
 * @brief 解析BO_消息行
 * @param line 行内容 "BO_ <id> <name>: <dlc> <transmitter>"
 * @return true=解析成功
 */
bool DbcParser::parseMessageLine(const QString& line)
{
    /* BO_ 1234 MsgName: 8 NodeName */
    static const QRegularExpression re(
        "^BO_\\s+(\\d+)\\s+(\\w+)\\s*:\\s*(\\d+)\\s+(\\w+)");
    const auto match = re.match(line);
    if (!match.hasMatch()) {
        return false;
    }

    DbcMessage msg;
    msg.id = match.captured(1).toUInt();
    msg.name = match.captured(2);
    msg.dlc = match.captured(3).toInt();
    msg.transmitter = match.captured(4);

    m_messages[msg.id] = msg;
    m_nameToId[msg.name] = msg.id;
    ++m_totalMessagesParsed;
    return true;
}

/**
 * @brief 解析SG_信号行
 * @param line 行内容
 * @param currentMsgId 当前所属消息ID
 * @return true=解析成功
 *
 * 格式: SG_ <name> [M|m<receiver>]: <startBit>|<bitLength>@<byteOrder><sign>
 *       (<factor>,<offset>) [<min>|<max>] "<unit>" <receiver>
 */
bool DbcParser::parseSignalLine(const QString& line, uint32_t currentMsgId)
{
    if (!m_messages.contains(currentMsgId)) {
        return false;
    }

    /*
     * 简化正则: 支持多路复用(M|m<receiver>)和普通信号
     * SG_ SignalName : 0|8@1+ (1,0) [0|255] "m" Receiver
     * SG_ SignalName M : 0|8@1+ (1,0) [0|255] "m" Receiver
     * SG_ SignalName m0 : 0|8@1+ (1,0) [0|255] "m" Receiver
     */
    static const QRegularExpression re(
        "^SG_\\s+(\\w+)\\s+(?:M|m\\d+\\s+)?:\\s*"
        "(\\d+)\\|(\\d+)@([01])([+-])\\s*"
        "\\(([^,]+),([^)]+)\\)\\s*"
        "\\[([^|]+)\\|([^\\]]+)\\]\\s*"
        "\"([^\"]*)\"\\s*"
        "(\\w+)");

    const auto match = re.match(line);
    if (!match.hasMatch()) {
        return false;
    }

    DbcSignal sig;
    sig.name = match.captured(1);
    sig.startBit = match.captured(2).toInt();
    sig.bitLength = match.captured(3).toInt();
    sig.byteOrder = match.captured(4).toInt();  /* 1=Intel(LE), 0=Motorola(BE) */
    /* match.captured(5) = 符号(+/-)，不存储但影响物理值范围解释 */
    sig.factor = match.captured(6).toDouble();
    sig.offset = match.captured(7).toDouble();
    sig.minimum = match.captured(8).toDouble();
    sig.maximum = match.captured(9).toDouble();
    sig.unit = match.captured(10);
    sig.receiver = match.captured(11);

    m_messages[currentMsgId].signalList.append(sig);
    return true;
}

/**
 * @brief 解析VAL_值表行
 * @param line 行内容 "VAL_ <id> <signalName> <val> "<desc>" ... ;"
 * @return true=解析成功
 */
bool DbcParser::parseValueTableLine(const QString& line)
{
    /* VAL_ 1234 SignalName 0 "Off" 1 "On" ; */
    static const QRegularExpression re("^VAL_\\s+(\\d+)\\s+(\\w+)\\s+(.+);");
    const auto match = re.match(line);
    if (!match.hasMatch()) {
        return false;
    }

    const uint32_t msgId = match.captured(1).toUInt();
    const QString sigName = match.captured(2);
    const QString pairs = match.captured(3).trimmed();

    if (!m_messages.contains(msgId)) {
        return false;
    }

    /* 查找对应信号 */
    DbcMessage& msg = m_messages[msgId];
    DbcSignal* target = nullptr;
    for (DbcSignal& sig : msg.signalList) {
        if (sig.name == sigName) {
            target = &sig;
            break;
        }
    }
    if (!target) {
        return false;
    }

    /* 解析 "值 描述" 对 */
    static const QRegularExpression pairRe("(\\d+)\\s+\"([^\"]*)\"");
    auto it = pairRe.globalMatch(pairs);
    while (it.hasNext()) {
        const auto m = it.next();
        const int val = m.captured(1).toInt();
        const QString desc = m.captured(2);
        target->valueTable[val] = desc;
    }

    return true;
}

/**
 * @brief 解析CM_注释行
 * @param line 行内容
 * @return true=解析成功
 */
bool DbcParser::parseCommentLine(const QString& line)
{
    /* CM_ BO_ 1234 "注释内容"; */
    static const QRegularExpression msgCommentRe(
        "^CM_\\s+BO_\\s+(\\d+)\\s+\"([^\"]*)\"\\s*;");
    auto match = msgCommentRe.match(line);
    if (match.hasMatch()) {
        const uint32_t msgId = match.captured(1).toUInt();
        if (m_messages.contains(msgId)) {
            m_messages[msgId].comment = match.captured(2);
        }
        return true;
    }

    /* CM_ SG_ 1234 SignalName "注释内容"; */
    static const QRegularExpression sigCommentRe(
        "^CM_\\s+SG_\\s+(\\d+)\\s+(\\w+)\\s+\"([^\"]*)\"\\s*;");
    match = sigCommentRe.match(line);
    /* 信号注释暂不存储到结构体中(可扩展) */
    Q_UNUSED(match)

    return false;
}

/**
 * @brief 解析BA_属性行
 * @param line 行内容 "BA_ "<attrName>" BO_ <id> <value>;"
 * @return true=解析成功
 */
bool DbcParser::parseAttributeLine(const QString& line)
{
    /* BA_ "GenMsgCycleTime" BO_ 1234 100; */
    static const QRegularExpression re(
        "^BA_\\s+\"([^\"]+)\"\\s+BO_\\s+(\\d+)\\s+(\\w+)\\s*;");
    const auto match = re.match(line);
    if (!match.hasMatch()) {
        return false;
    }

    const QString attrName = match.captured(1);
    const uint32_t msgId = match.captured(2).toUInt();
    const QString attrValue = match.captured(3);

    if (m_messages.contains(msgId)) {
        m_messages[msgId].attributes[attrName] = attrValue;
    }

    return true;
}

/**
 * @brief 解析BU_节点行
 * @param line 行内容 "BU_: Node1 Node2 Node3"
 * @return true=解析成功
 */
bool DbcParser::parseNodeLine(const QString& line)
{
    /* BU_: Node1 Node2 Node3 */
    static const QRegularExpression re("^BU_:\\s*(.+)$");
    const auto match = re.match(line);
    if (!match.hasMatch()) {
        return false;
    }

    const QString nodesPart = match.captured(1).trimmed();
    m_nodes = nodesPart.split(' ', Qt::SkipEmptyParts);
    return true;
}

// ──────────────────────── 私有: 位域提取 ────────────────────────

/**
 * @brief 从字节数据中提取指定位域的原始值
 * @param data 帧数据(最多8字节)
 * @param startBit 起始位(DBC编号)
 * @param bitLength 位长度
 * @param byteOrder 字节序(1=Intel小端, 0=Motorola大端)
 * @return 提取的原始值
 */
uint64_t DbcParser::extractBits(const QByteArray& data, int startBit,
                                 int bitLength, int byteOrder) const
{
    const int dataLen = data.size();
    if (dataLen == 0 || bitLength <= 0 || bitLength > 64) {
        return 0;
    }

    uint64_t result = 0;

    if (byteOrder == 1) {
        /* Intel / 小端(LSB): 位序从低位到高位顺序排列
         * startBit是最低位编号，连续向高位取bitLength位 */
        for (int i = 0; i < bitLength; ++i) {
            const int bitPos = startBit + i;
            const int byteIdx = bitPos / 8;
            const int bitIdx = bitPos % 8;
            if (byteIdx < dataLen) {
                const uint8_t byteVal = static_cast<uint8_t>(data[byteIdx]);
                if (byteVal & (1 << bitIdx)) {
                    result |= (1ULL << i);
                }
            }
        }
    } else {
        /* Motorola / 大端(MSB): DBC中使用翻转位编号
         * 需要将DBC的startBit转换为实际字节/位位置 */
        for (int i = 0; i < bitLength; ++i) {
            /* Motorola位编号: 每字节内位序翻转 */
            int bitPos = startBit - i;
            if (bitPos < 0) {
                break;
            }
            const int byteIdx = bitPos / 8;
            /* Motorola: 在字节内，位7是MSB，位0是LSB
             * DBC编号: 位8n+7是字节n的最高位 */
            const int bitIdx = 7 - (bitPos % 8);
            if (byteIdx < dataLen) {
                const uint8_t byteVal = static_cast<uint8_t>(data[byteIdx]);
                if (byteVal & (1 << bitIdx)) {
                    result |= (1ULL << (bitLength - 1 - i));
                }
            }
        }
    }

    return result;
}

quint64 DbcParser::totalParses() const { return m_totalParses; }
quint64 DbcParser::totalMessagesParsed() const { return m_totalMessagesParsed; }
quint64 DbcParser::totalSignalsDecoded() const { return m_totalSignalsDecoded; }
quint64 DbcParser::totalParseErrors() const { return m_totalParseErrors; }

void DbcParser::resetDbcStatistics()
{
    m_totalParses = 0;
    m_totalMessagesParsed = 0;
    m_totalSignalsDecoded = 0;
    m_totalParseErrors = 0;
}
