/**
 * @file ByteStreamParser.h
 * @brief 字节流解析器 -- 基于状态机的增量式协议帧解析引擎
 *
 * 支持: 定界符检测/长度字段解析/转义序列(HDLC风格)/超时/多并发上下文/增量喂入。
 * 每个 ParseContext 维护独立状态机，ByteStreamParser 提供统一 feed 入口。
 */

#ifndef BYTESTREAMPARSER_H
#define BYTESTREAMPARSER_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QTimer>
#include <QUuid>

/** @brief 字节流解析器 -- 状态机驱动的增量帧解析，支持多协议并发解析上下文 */
class ByteStreamParser : public QObject {
    Q_OBJECT

public:
    enum class Endianness { LittleEndian, BigEndian }; ///< 字节序枚举
    Q_ENUM(Endianness)

    enum class ParseState {
        Idle, InFrame, EscapeSequence, LengthField, AwaitingEnd
    };
    Q_ENUM(ParseState)

    /** @brief 运行时统计快照 */
    struct Stats {
        quint64 totalBytesFed          = 0;  ///< 累计喂入字节总数
        quint64 totalFramesParsed      = 0;  ///< 成功解析帧总数
        quint64 totalIncompleteFrames  = 0;  ///< 未完成帧数(超时/溢出)
        quint64 totalCrcFails          = 0;  ///< CRC校验失败总数
        quint64 totalEscapeSequences   = 0;  ///< 处理的转义序列总数
        quint64 totalFrameBytes        = 0;  ///< 累计帧字节总数
        double  avgFrameSize           = 0.0;///< 平均帧大小(字节)
        quint64 parseErrors            = 0;  ///< 解析错误总数
    };

    explicit ByteStreamParser(QObject *parent = nullptr);
    ~ByteStreamParser() override;

    // ---- 上下文管理 ----
    QUuid createContext();                     ///< 创建新解析上下文，返回唯一标识
    bool removeContext(const QUuid &id);       ///< 移除指定上下文
    int contextCount() const;                  ///< 活跃上下文数量

    // ---- 配置(应用于指定上下文，id=空时使用最近创建的) ----
    void setDelimiter(const QByteArray &start, const QByteArray &end,
                      const QUuid &id = QUuid());            ///< 设置起始/结束定界符
    void setLengthField(int offset, int length, Endianness endian,
                        const QUuid &id = QUuid());          ///< 设置长度字段(offset/字节数/字节序)
    void setEscapeByte(char escape, char xorByte,
                       const QUuid &id = QUuid());           ///< 设置转义字节对(如HDLC 0x7D/0x20)
    void setTimeout(int timeoutMs,
                    const QUuid &id = QUuid());              ///< 设置帧超时(ms，0=禁用)
    void setMaxFrameLength(int maxLen,
                           const QUuid &id = QUuid());       ///< 设置最大帧长度(硬上限8192)

    // ---- 喂入数据 ----
    void feed(const QByteArray &data);         ///< 喂入原始字节流，分发给所有上下文

    // ---- 统计 ----
    Stats stats() const;                       ///< 获取聚合统计快照
    void resetStatistics();                    ///< 重置所有上下文统计计数器

signals:
    void frameParsed(const QByteArray &frame, const QUuid &contextId);  ///< 帧解析完成(已去转义)
    void partialFrame(int bytesSoFar, const QUuid &contextId);          ///< 部分帧进度
    void parseError(const QString &errorMessage, const QUuid &contextId); ///< 解析错误

private:
    /** @brief 单个解析上下文 -- 独立状态机实例+配置+统计 */
    struct ParseContext {
        QUuid id;                                       ///< 上下文唯一标识
        QByteArray startDelimiter;                      ///< 起始定界符(空=无起始检测)
        QByteArray endDelimiter;                        ///< 结束定界符(空=无结束检测)
        int lengthFieldOffset = -1;                     ///< 长度字段偏移(-1=未设置)
        int lengthFieldSize  = 0;                       ///< 长度字段字节数
        Endianness lengthEndianness = Endianness::LittleEndian;
        char escapeByte  = 0x00;                        ///< 转义标记(0=未启用)
        char xorByte     = 0x00;                        ///< 转义XOR值
        int timeoutMs    = 0;                           ///< 帧超时(ms)
        int maxFrameLen  = 4096;                        ///< 最大帧长度

        ParseState state = ParseState::Idle;
        QByteArray frameBuffer;                         ///< 当前帧缓冲区
        int startMatchPos  = 0;                         ///< 起始定界符匹配进度
        int endMatchPos    = 0;                         ///< 结束定界符匹配进度
        int lengthBytesReceived = 0;                    ///< 已接收长度字段字节数
        int expectedPayload = 0;                        ///< 期望载荷长度
        bool escapeActive  = false;                     ///< 是否处于转义序列中
        QElapsedTimer frameTimer;
        QTimer *timeoutTimer = nullptr;

        quint64 totalBytesFed = 0, totalFramesParsed = 0;
        quint64 totalIncompleteFrames = 0, totalCrcFails = 0;
        quint64 totalEscapeSequences = 0, totalFrameBytes = 0;
        quint64 parseErrors = 0;
    };

    void processByte(ParseContext &ctx, unsigned char byte); ///< 逐字节状态机入口
    void completeFrame(ParseContext &ctx);                   ///< 完成帧解析(去转义+信号+统计)
    void resetContext(ParseContext &ctx);                    ///< 重置到Idle(保留配置)
    void handleTimeout(ParseContext &ctx);                   ///< 处理帧超时
    QByteArray unescapeFrame(const QByteArray &raw, char esc,
                             char xorVal) const;             ///< 对帧数据执行去转义
    int parseLengthValue(const QByteArray &data, int off,
                         int sz, Endianness end) const;      ///< 解析长度字段值
    ParseContext *findContext(const QUuid &id);               ///< 按id查找上下文
    void startTimeout(ParseContext &ctx);                    ///< 启动超时定时器(延迟创建)
    void stopTimeout(ParseContext &ctx);                     ///< 停止超时定时器

    QList<ParseContext> m_contexts;  ///< 解析上下文列表
    QUuid m_lastContextId;           ///< 最近创建的上下文ID
    static constexpr int kHardMaxFrameLen = 8192;
};

#endif // BYTESTREAMPARSER_H
