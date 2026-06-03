#include "protocol/analyzer/ProtocolAnalyzer.h"
ProtocolAnalyzer::ProtocolAnalyzer(QObject *parent) : QObject(parent) {}
ProtocolAnalyzer::~ProtocolAnalyzer() = default;
void ProtocolAnalyzer::setProtocolTemplate(const QString &name, const QList<Field> &tpl) { m_templates[name] = tpl; }
void ProtocolAnalyzer::removeTemplate(const QString &name) { m_templates.remove(name); }
ProtocolAnalyzer::ProtocolMessage ProtocolAnalyzer::parse(const QByteArray &data, const QString &tplName) {
    ProtocolMessage msg; msg.rawData = data; msg.protocolName = tplName;
    auto it = m_templates.constFind(tplName);
    if (it == m_templates.constEnd()) { msg.valid = false; msg.errorMessage = tr("Template not found: %1").arg(tplName); emit parseError(tplName, msg.errorMessage); return msg; }
    int byteOff = 0;
    for (const auto &field : it.value()) {
        Field parsed = field;
        int bytesNeeded = (parsed.bitLength + 7) / 8;
        if (byteOff + bytesNeeded > data.size()) { msg.valid = false; msg.errorMessage = tr("Data too short for: %1").arg(parsed.name); emit parseError(tplName, msg.errorMessage); return msg; }
        parsed.value = data.mid(byteOff, bytesNeeded);
        parsed.displayValue = parsed.value.toHex(' ').toUpper();
        byteOff += bytesNeeded;
        msg.fields.append(parsed);
    }
    msg.valid = true;
    emit messageParsed(msg);
    return msg;
}
QList<ProtocolAnalyzer::ProtocolMessage> ProtocolAnalyzer::parseStream(const QByteArray &data, const QString &tplName, int frameSize) {
    QList<ProtocolMessage> results; int offset = 0;
    while (offset + frameSize <= data.size()) { results.append(parse(data.mid(offset, frameSize), tplName)); offset += frameSize; }
    return results;
}
QStringList ProtocolAnalyzer::templates() const { return m_templates.keys(); }
void ProtocolAnalyzer::clearTemplates() { m_templates.clear(); }
