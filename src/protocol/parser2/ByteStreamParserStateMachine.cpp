/**
 * @file ByteStreamParserStateMachine.cpp
 * @brief 字节流解析器 — 状态机核心实现
 *
 * 从 ByteStreamParser.cpp 拆分而来，包含:
 *   - processByte(): 逐字节状态转移(Idle/InFrame/EscapeSequence/LengthField/AwaitingEnd)
 *   - completeFrame(): 帧完成处理(去转义+发射信号)
 *   - resetContext(): 重置上下文状态
 *   - handleTimeout(): 超时处理
 *   - unescapeFrame(): 去转义辅助
 *   - parseLengthValue(): 长度字段解析
 *   - findContext(): 上下文查找
 *   - startTimeout()/stopTimeout(): 超时定时器管理
 */

#include "protocol/parser2/ByteStreamParser.h"

// ---- 状态机核心 ----

void ByteStreamParser::processByte(ParseContext &ctx, unsigned char byte)
{
    if (ctx.state != ParseState::Idle && ctx.timeoutMs > 0 && ctx.frameTimer.isValid())
        ctx.frameTimer.restart();

    switch (ctx.state) {
    case ParseState::Idle: {
        if (ctx.startDelimiter.isEmpty()) {
            ctx.frameBuffer.append(static_cast<char>(byte));
            ctx.state = ParseState::InFrame;
            startTimeout(ctx);
        } else if (byte == static_cast<unsigned char>(ctx.startDelimiter[ctx.startMatchPos])) {
            ++ctx.startMatchPos;
            if (ctx.startMatchPos >= ctx.startDelimiter.size()) {
                ctx.frameBuffer = ctx.startDelimiter;
                ctx.startMatchPos = 0;
                ctx.state = ParseState::InFrame;
                startTimeout(ctx);
            }
        } else {
            ctx.startMatchPos = 0;
        }
        break;
    }

    case ParseState::InFrame: {
        if (ctx.escapeByte != 0x00 && byte == static_cast<unsigned char>(ctx.escapeByte)) {
            ctx.escapeActive = true;
            ctx.state = ParseState::EscapeSequence;
            break;
        }
        if (ctx.lengthFieldOffset >= 0 && ctx.lengthFieldSize > 0
            && ctx.frameBuffer.size() == ctx.lengthFieldOffset
            && ctx.expectedPayload == 0) {
            ctx.frameBuffer.append(static_cast<char>(byte));
            ctx.lengthBytesReceived = 1;
            ctx.state = ParseState::LengthField;
            break;
        }
        if (!ctx.endDelimiter.isEmpty()) {
            if (byte == static_cast<unsigned char>(ctx.endDelimiter[ctx.endMatchPos])) {
                ++ctx.endMatchPos;
                if (ctx.endMatchPos >= ctx.endDelimiter.size()) {
                    ctx.frameBuffer.append(ctx.endDelimiter);
                    completeFrame(ctx);
                    return;
                }
                break;
            }
            if (ctx.endMatchPos > 0) {
                ctx.frameBuffer.append(ctx.endDelimiter.left(ctx.endMatchPos));
                ctx.endMatchPos = 0;
            }
        }
        ctx.frameBuffer.append(static_cast<char>(byte));
        if (ctx.frameBuffer.size() > ctx.maxFrameLen) {
            ++ctx.parseErrors; ++ctx.totalIncompleteFrames;
            emit parseError(tr("帧长度超过上限 %1 字节").arg(ctx.maxFrameLen), ctx.id);
            resetContext(ctx);
            return;
        }
        if (ctx.endDelimiter.isEmpty() && ctx.expectedPayload > 0) {
            int payloadStart = ctx.lengthFieldOffset + ctx.lengthFieldSize;
            if (ctx.frameBuffer.size() - payloadStart >= ctx.expectedPayload) {
                completeFrame(ctx);
                return;
            }
        }
        emit partialFrame(ctx.frameBuffer.size(), ctx.id);
        break;
    }

    case ParseState::EscapeSequence:
        ctx.frameBuffer.append(static_cast<char>(byte ^ static_cast<unsigned char>(ctx.xorByte)));
        ++ctx.totalEscapeSequences;
        ctx.escapeActive = false;
        ctx.state = ParseState::InFrame;
        break;

    case ParseState::LengthField:
        ctx.frameBuffer.append(static_cast<char>(byte));
        ++ctx.lengthBytesReceived;
        if (ctx.lengthBytesReceived >= ctx.lengthFieldSize) {
            ctx.expectedPayload = parseLengthValue(
                ctx.frameBuffer, ctx.lengthFieldOffset,
                ctx.lengthFieldSize, ctx.lengthEndianness);
            ctx.lengthBytesReceived = 0;
            if (ctx.expectedPayload < 0 || ctx.expectedPayload > ctx.maxFrameLen) {
                ++ctx.parseErrors; ++ctx.totalIncompleteFrames;
                emit parseError(tr("无效长度字段值: %1").arg(ctx.expectedPayload), ctx.id);
                resetContext(ctx);
                return;
            }
            ctx.state = ctx.endDelimiter.isEmpty() ? ParseState::InFrame : ParseState::AwaitingEnd;
        }
        break;

    case ParseState::AwaitingEnd: {
        if (ctx.escapeByte != 0x00 && byte == static_cast<unsigned char>(ctx.escapeByte)) {
            ctx.escapeActive = true;
            ctx.state = ParseState::EscapeSequence;
            break;
        }
        if (!ctx.endDelimiter.isEmpty()) {
            if (byte == static_cast<unsigned char>(ctx.endDelimiter[ctx.endMatchPos])) {
                ++ctx.endMatchPos;
                if (ctx.endMatchPos >= ctx.endDelimiter.size()) {
                    ctx.frameBuffer.append(ctx.endDelimiter);
                    completeFrame(ctx);
                    return;
                }
                break;
            }
            if (ctx.endMatchPos > 0) {
                ctx.frameBuffer.append(ctx.endDelimiter.left(ctx.endMatchPos));
                ctx.endMatchPos = 0;
            }
        }
        ctx.frameBuffer.append(static_cast<char>(byte));
        if (ctx.frameBuffer.size() > ctx.maxFrameLen) {
            ++ctx.parseErrors; ++ctx.totalIncompleteFrames;
            emit parseError(tr("帧长度超过上限 %1 字节").arg(ctx.maxFrameLen), ctx.id);
            resetContext(ctx);
            return;
        }
        emit partialFrame(ctx.frameBuffer.size(), ctx.id);
        break;
    }
    }
}

// ---- 帧完成 / 重置 / 超时 ----

void ByteStreamParser::completeFrame(ParseContext &ctx)
{
    QByteArray finalFrame = ctx.frameBuffer;
    if (ctx.escapeByte != 0x00)
        finalFrame = unescapeFrame(finalFrame, ctx.escapeByte, ctx.xorByte);
    ++ctx.totalFramesParsed;
    ctx.totalFrameBytes += static_cast<quint64>(finalFrame.size());
    stopTimeout(ctx);
    emit frameParsed(finalFrame, ctx.id);
    resetContext(ctx);
}

void ByteStreamParser::resetContext(ParseContext &ctx)
{
    ctx.state = ParseState::Idle;
    ctx.frameBuffer.clear();
    ctx.startMatchPos = 0;
    ctx.endMatchPos = 0;
    ctx.lengthBytesReceived = 0;
    ctx.expectedPayload = 0;
    ctx.escapeActive = false;
    stopTimeout(ctx);
}

void ByteStreamParser::handleTimeout(ParseContext &ctx)
{
    if (ctx.frameBuffer.isEmpty()) return;
    ++ctx.totalIncompleteFrames; ++ctx.parseErrors;
    emit parseError(
        tr("帧接收超时(%1ms)，已收集 %2 字节").arg(ctx.timeoutMs).arg(ctx.frameBuffer.size()),
        ctx.id);
    resetContext(ctx);
}

// ---- 辅助方法 ----

QByteArray ByteStreamParser::unescapeFrame(const QByteArray &raw,
                                            char escapeByte, char xorByte) const
{
    QByteArray result;
    result.reserve(raw.size());
    const auto esc = static_cast<unsigned char>(escapeByte);
    const auto xorVal = static_cast<unsigned char>(xorByte);
    bool inEscape = false;
    for (int i = 0; i < raw.size(); ++i) {
        const auto b = static_cast<unsigned char>(raw[i]);
        if (inEscape) {
            result.append(static_cast<char>(b ^ xorVal));
            inEscape = false;
        } else if (b == esc) {
            inEscape = true;
        } else {
            result.append(raw[i]);
        }
    }
    if (inEscape) result.append(escapeByte);
    return result;
}

int ByteStreamParser::parseLengthValue(const QByteArray &data, int offset,
                                        int size, Endianness endian) const
{
    if (offset < 0 || size <= 0 || offset + size > data.size()) return -1;
    quint32 value = 0;
    if (endian == Endianness::BigEndian) {
        for (int i = 0; i < size; ++i)
            value = (value << 8) | static_cast<quint8>(data[offset + i]);
    } else {
        for (int i = size - 1; i >= 0; --i)
            value = (value << 8) | static_cast<quint8>(data[offset + i]);
    }
    return static_cast<int>(value);
}

ByteStreamParser::ParseContext *ByteStreamParser::findContext(const QUuid &id)
{
    for (auto &ctx : m_contexts) {
        if (ctx.id == id) return &ctx;
    }
    return nullptr;
}

// ---- 超时定时器管理 ----

void ByteStreamParser::startTimeout(ParseContext &ctx)
{
    if (ctx.timeoutMs <= 0) return;
    if (!ctx.timeoutTimer) {
        ctx.timeoutTimer = new QTimer(this);
        ctx.timeoutTimer->setSingleShot(true);
        QUuid cid = ctx.id;
        connect(ctx.timeoutTimer, &QTimer::timeout, this, [this, cid]() {
            auto *c = findContext(cid);
            if (c) handleTimeout(*c);
        });
    }
    ctx.timeoutTimer->start(ctx.timeoutMs);
    ctx.frameTimer.start();
}

void ByteStreamParser::stopTimeout(ParseContext &ctx)
{
    if (ctx.timeoutTimer) ctx.timeoutTimer->stop();
    ctx.frameTimer.invalidate();
}
