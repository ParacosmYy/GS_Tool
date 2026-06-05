/**
 * @file ProtocolFrameValidator.cpp
 * @brief 协议帧验证器实现 -- 头尾/长度/CRC/字段范围四重校验
 */

#include "protocol/validator/ProtocolFrameValidator.h"

ProtocolFrameValidator::ProtocolFrameValidator(QObject* parent) : QObject(parent)
{
    setObjectName(QStringLiteral("ProtocolFrameValidator"));
}

void ProtocolFrameValidator::setRule(const FrameRule& rule) { m_rule = rule; }
const ProtocolFrameValidator::FrameRule& ProtocolFrameValidator::rule() const { return m_rule; }
void ProtocolFrameValidator::clearRule() { m_rule = FrameRule{}; }

QList<ProtocolFrameValidator::ValidationIssue> ProtocolFrameValidator::validate(
    const QByteArray& frame)
{
    QList<ValidationIssue> issues;
    ++m_stats.totalFramesValidated;

    if (!checkHeaderFooter(frame)) {
        issues.append({tr("帧头/帧尾不匹配"), ValidationSeverity::Error, 0, {}});
        ++m_stats.totalPatternErrors;
    }
    if (!checkLength(frame)) {
        issues.append({tr("帧长度超出范围(%1~%2)")
            .arg(m_rule.minLength).arg(m_rule.maxLength), ValidationSeverity::Error, 0, {}});
        ++m_stats.totalLengthErrors;
    }
    if (!checkCrc(frame)) {
        issues.append({tr("CRC校验失败"), ValidationSeverity::Error,
            qMax(0, m_rule.crcOffset), {}});
        ++m_stats.totalCrcErrors;
    }
    if (!checkFieldRanges(frame)) {
        issues.append({tr("字段值超出允许范围"), ValidationSeverity::Warning, 0, {}});
        ++m_stats.totalFieldRangeErrors;
    }

    bool valid = issues.isEmpty();
    if (valid) ++m_stats.totalValidFrames;
    else ++m_stats.totalInvalidFrames;

    emit frameValidated(frame, valid);
    if (!valid) emit validationFailed(frame, issues);
    return issues;
}

bool ProtocolFrameValidator::isValid(const QByteArray& frame)
{
    return validate(frame).isEmpty();
}

bool ProtocolFrameValidator::checkHeaderFooter(const QByteArray& frame) const
{
    if (frame.isEmpty()) return false;
    if (!m_rule.headerPattern.isEmpty()) {
        if (!frame.startsWith(m_rule.headerPattern)) return false;
    }
    if (!m_rule.footerPattern.isEmpty()) {
        if (!frame.endsWith(m_rule.footerPattern)) return false;
    }
    return true;
}

bool ProtocolFrameValidator::checkLength(const QByteArray& frame) const
{
    int len = frame.size();
    return len >= m_rule.minLength && len <= m_rule.maxLength;
}

bool ProtocolFrameValidator::checkCrc(const QByteArray& frame) const
{
    if (m_rule.crcOffset < 0 || m_rule.crcAlgorithm.isEmpty()) return true;
    if (m_rule.crcOffset + m_rule.crcLength > frame.size()) return false;

    /* 提取期望CRC值 */
    quint32 expected = 0;
    for (int i = 0; i < m_rule.crcLength; ++i)
        expected = (expected << 8) | static_cast<quint8>(frame[m_rule.crcOffset + i]);

    /* 计算区域: 头部到CRC之前 */
    QByteArray data = frame.left(m_rule.crcOffset);
    if (m_rule.footerPattern.isEmpty() == false && frame.endsWith(m_rule.footerPattern))
        data = frame.left(frame.size() - m_rule.footerPattern.size());
    data = data.left(m_rule.crcOffset);

    quint32 computed = computeCrc(data, m_rule.crcAlgorithm);
    return computed == expected;
}

bool ProtocolFrameValidator::checkFieldRanges(const QByteArray& frame) const
{
    for (auto it = m_rule.fieldRanges.constBegin(); it != m_rule.fieldRanges.constEnd(); ++it) {
        int offset = it.value().first;
        int maxVal = it.value().second;
        if (offset < 0 || offset >= frame.size()) return false;
        int val = static_cast<quint8>(frame[offset]);
        if (val > maxVal) return false;
    }
    return true;
}

quint32 ProtocolFrameValidator::computeCrc(const QByteArray& data, const QString& algo) const
{
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data.constData());
    int len = data.size();
    quint32 result = 0;

    if (algo == QStringLiteral("xor8")) {
        uint8_t crc = 0;
        for (int i = 0; i < len; ++i) crc ^= p[i];
        result = crc;
    } else if (algo == QStringLiteral("sum8")) {
        uint8_t sum = 0;
        for (int i = 0; i < len; ++i) sum += p[i];
        result = sum;
    } else if (algo == QStringLiteral("crc8")) {
        uint8_t crc = 0x00;
        for (int i = 0; i < len; ++i) {
            crc ^= p[i];
            for (int j = 0; j < 8; ++j)
                crc = (crc & 0x80) ? ((crc << 1) ^ 0x07) : (crc << 1);
        }
        result = crc;
    } else if (algo == QStringLiteral("crc16")) {
        uint16_t crc = 0xFFFF;
        for (int i = 0; i < len; ++i) {
            crc ^= static_cast<uint16_t>(p[i]);
            for (int j = 0; j < 8; ++j)
                crc = (crc & 1) ? ((crc >> 1) ^ 0xA001) : (crc >> 1);
        }
        result = crc;
    } else if (algo == QStringLiteral("crc32")) {
        uint32_t crc = 0xFFFFFFFF;
        for (int i = 0; i < len; ++i) {
            crc ^= p[i];
            for (int j = 0; j < 8; ++j)
                crc = (crc & 1) ? ((crc >> 1) ^ 0xEDB88320u) : (crc >> 1);
        }
        result = crc ^ 0xFFFFFFFF;
    }
    return result;
}

ProtocolFrameValidator::Stats ProtocolFrameValidator::stats() const { return m_stats; }
void ProtocolFrameValidator::resetStatistics() { m_stats = Stats{}; }
