/**
 * @file ProtocolFrameValidator.h
 * @brief 协议帧验证器 -- 校验帧头尾/长度/CRC/字段范围
 */

#ifndef PROTOCOLFRAMEVALIDATOR_H
#define PROTOCOLFRAMEVALIDATOR_H

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QString>

class ProtocolFrameValidator : public QObject {
    Q_OBJECT

public:
    enum class ValidationSeverity { Error, Warning, Info };

    struct ValidationIssue {
        QString message;
        ValidationSeverity severity = ValidationSeverity::Error;
        int offset = 0;
        QString fieldName;
    };

    struct FrameRule {
        QByteArray headerPattern;
        QByteArray footerPattern;
        int minLength = 0;
        int maxLength = 65535;
        int crcOffset = -1;
        int crcLength = 1;
        QString crcAlgorithm;
        QMap<QString, QPair<int, int>> fieldRanges;
    };

    struct Stats {
        quint64 totalFramesValidated = 0;
        quint64 totalValidFrames = 0;
        quint64 totalInvalidFrames = 0;
        quint64 totalCrcErrors = 0;
        quint64 totalLengthErrors = 0;
        quint64 totalPatternErrors = 0;
        quint64 totalFieldRangeErrors = 0;
    };

    explicit ProtocolFrameValidator(QObject* parent = nullptr);

    void setRule(const FrameRule& rule);
    const FrameRule& rule() const;
    QList<ValidationIssue> validate(const QByteArray& frame);
    bool isValid(const QByteArray& frame);
    void clearRule();
    Stats stats() const;
    void resetStatistics();

signals:
    void frameValidated(const QByteArray& frame, bool valid);
    void validationFailed(const QByteArray& frame, const QList<ValidationIssue>& issues);

private:
    FrameRule m_rule;
    mutable Stats m_stats;
    bool checkHeaderFooter(const QByteArray& frame) const;
    bool checkLength(const QByteArray& frame) const;
    bool checkCrc(const QByteArray& frame) const;
    bool checkFieldRanges(const QByteArray& frame) const;
    quint32 computeCrc(const QByteArray& data, const QString& algo) const;
};

#endif // PROTOCOLFRAMEVALIDATOR_H
