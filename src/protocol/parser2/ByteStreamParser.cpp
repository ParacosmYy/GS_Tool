/**
 * @file ByteStreamParser.cpp
 * @brief 字节流解析器实现 — 上下文管理/配置/喂入/统计
 *
 * 状态机核心见 ByteStreamParserStateMachine.cpp。
 */

#include "protocol/parser2/ByteStreamParser.h"

// ---- 构造 / 析构 ----

ByteStreamParser::ByteStreamParser(QObject *parent) : QObject(parent)
{
    setObjectName(QStringLiteral("ByteStreamParser"));
}

ByteStreamParser::~ByteStreamParser()
{
    for (auto &ctx : m_contexts) {
        if (ctx.timeoutTimer) { ctx.timeoutTimer->stop(); delete ctx.timeoutTimer; }
    }
}

// ---- 上下文管理 ----

QUuid ByteStreamParser::createContext()
{
    ParseContext ctx;
    ctx.id = QUuid::createUuid();
    m_lastContextId = ctx.id;
    m_contexts.append(std::move(ctx));
    return m_lastContextId;
}

bool ByteStreamParser::removeContext(const QUuid &id)
{
    for (int i = 0; i < m_contexts.size(); ++i) {
        if (m_contexts[i].id == id) {
            if (m_contexts[i].timeoutTimer) {
                m_contexts[i].timeoutTimer->stop();
                delete m_contexts[i].timeoutTimer;
            }
            m_contexts.removeAt(i);
            return true;
        }
    }
    return false;
}

int ByteStreamParser::contextCount() const { return m_contexts.size(); }

// ---- 配置 ----

void ByteStreamParser::setDelimiter(const QByteArray &start,
                                     const QByteArray &end, const QUuid &id)
{
    auto *ctx = findContext(id.isNull() ? m_lastContextId : id);
    if (!ctx) return;
    ctx->startDelimiter = start;
    ctx->endDelimiter = end;
}

void ByteStreamParser::setLengthField(int offset, int length,
                                       Endianness endian, const QUuid &id)
{
    auto *ctx = findContext(id.isNull() ? m_lastContextId : id);
    if (!ctx) return;
    ctx->lengthFieldOffset = offset;
    ctx->lengthFieldSize = qBound(0, length, 4);
    ctx->lengthEndianness = endian;
}

void ByteStreamParser::setEscapeByte(char escape, char xorByte,
                                      const QUuid &id)
{
    auto *ctx = findContext(id.isNull() ? m_lastContextId : id);
    if (!ctx) return;
    ctx->escapeByte = escape;
    ctx->xorByte = xorByte;
}

void ByteStreamParser::setTimeout(int timeoutMs, const QUuid &id)
{
    auto *ctx = findContext(id.isNull() ? m_lastContextId : id);
    if (!ctx) return;
    ctx->timeoutMs = qMax(0, timeoutMs);
    if (ctx->timeoutMs == 0 && ctx->timeoutTimer) {
        ctx->timeoutTimer->stop();
        delete ctx->timeoutTimer;
        ctx->timeoutTimer = nullptr;
    }
}

void ByteStreamParser::setMaxFrameLength(int maxLen, const QUuid &id)
{
    auto *ctx = findContext(id.isNull() ? m_lastContextId : id);
    if (!ctx) return;
    ctx->maxFrameLen = qBound(1, maxLen, kHardMaxFrameLen);
}

// ---- 喂入数据 ----

void ByteStreamParser::feed(const QByteArray &data)
{
    if (data.isEmpty()) return;
    for (auto &ctx : m_contexts) {
        ctx.totalBytesFed += static_cast<quint64>(data.size());
        for (int i = 0; i < data.size(); ++i)
            processByte(ctx, static_cast<unsigned char>(data[i]));
    }
}

// ---- 统计 ----

ByteStreamParser::Stats ByteStreamParser::stats() const
{
    Stats s;
    for (const auto &c : m_contexts) {
        s.totalBytesFed += c.totalBytesFed;
        s.totalFramesParsed += c.totalFramesParsed;
        s.totalIncompleteFrames += c.totalIncompleteFrames;
        s.totalCrcFails += c.totalCrcFails;
        s.totalEscapeSequences += c.totalEscapeSequences;
        s.totalFrameBytes += c.totalFrameBytes;
        s.parseErrors += c.parseErrors;
    }
    s.avgFrameSize = (s.totalFramesParsed > 0)
        ? static_cast<double>(s.totalFrameBytes) / static_cast<double>(s.totalFramesParsed)
        : 0.0;
    return s;
}

void ByteStreamParser::resetStatistics()
{
    for (auto &c : m_contexts) {
        c.totalBytesFed = 0; c.totalFramesParsed = 0;
        c.totalIncompleteFrames = 0; c.totalCrcFails = 0;
        c.totalEscapeSequences = 0; c.totalFrameBytes = 0;
        c.parseErrors = 0;
    }
}
