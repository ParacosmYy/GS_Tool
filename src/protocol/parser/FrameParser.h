/** @file FrameParser.h @brief 帧解析状态机 - 从字节流中实时解析协议帧。状态: Idle->HeaderMatching->LengthReceiving->PayloadReceiving->ChecksumVerifying->FooterMatching->Done。安全: 帧长度上限/超时机制/硬性上限4096B */

#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>
#include <QTimer>
#include "protocol/parser/FrameDefinition.h"
#include "utils/crypto/CRC.h"

/** @brief 帧解析状态机。协作: ProtocolBridgeManager(数据路由)/FrameDefinition(帧结构)/ChartModel+ProtocolView(下游消费) */
class FrameParser : public QObject {
    Q_OBJECT

public:
    explicit FrameParser(QObject* parent = nullptr); ///< 构造(默认帧上限1024B)
    ~FrameParser() override;                 ///< 析构(释放超时定时器等)
    void setDefinition(const FrameDefinition& def); ///< 设置帧格式定义(自动reset)
    FrameDefinition definition() const;      ///< 获取当前帧格式定义
    void feed(const QByteArray& data);       ///< 喂入字节流(逐字节送入状态机)
    void reset();                            ///< 重置解析器状态(清空缓冲区, 不清零计数器)
    quint64 frameCount() const;              ///< 已解析帧计数(兼容旧接口)
    quint64 errorCount() const;              ///< 错误帧计数(兼容旧接口)
    void setMaxFrameLength(int maxLen);      ///< 设置帧长度上限(默认1024B, 硬上限4096B)
    int maxFrameLength() const;              ///< 获取帧长度上限
    void setFrameTimeout(int timeoutMs);     ///< 设置状态机超时阈值(ms, 0=禁用)
    int frameTimeout() const;                ///< 获取超时阈值(ms)

    /* -- 统计计数器接口 -- */
    quint64 totalFramesParsed() const;       ///< 成功解析帧总数
    quint64 totalBytesInput() const;         ///< 累计输入字节总数
    quint64 totalChecksumErrors() const;     ///< 校验和错误次数
    quint64 totalOverflows() const;          ///< 累计溢出次数(帧超最大长度)
    quint64 totalParseErrors() const;        ///< 累计解析错误次数
    quint64 totalBytesParsed() const;        ///< 成功解析帧中的有效数据字节总数
    quint64 totalSyncLost() const;           ///< 累计同步丢失次数(帧头匹配失败导致缓冲区清空)
    quint64 totalFramesBuilt() const;        ///< 累计帧构建完成次数(completeFrame调用)
    quint64 totalValidationErrors() const;   ///< 累计校验验证失败次数(CRC/校验和不匹配)
    quint64 totalAutoDetectCalls() const;    ///< 累计自动检测调用次数(为ProtocolBridgeManager预留)
    quint64 totalMalformedFrames() const;    ///< 累计畸形帧次数(帧头/帧尾/长度异常)
    double avgFrameSize() const;             ///< 平均帧大小(字节)
    void resetStats();                       ///< 重置所有统计计数器

signals:
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame); ///< 帧解析成功
    void frameError(const QString& reason, const QByteArray& rawFrame); ///< 帧解析错误

private:
    enum class State { Idle, HeaderMatching, LengthReceiving, PayloadReceiving, ChecksumVerifying, FooterMatching }; ///< 解析状态
    // ---- 状态处理方法 ----
    void processByte(unsigned char byte);    ///< 逐字节状态机入口
    void handleHeaderMatching(unsigned char byte); ///< 帧头匹配状态
    void handleLengthReceiving(unsigned char byte); ///< 长度字段接收状态
    void handlePayloadReceiving(unsigned char byte); ///< 有效载荷接收状态
    void processCompletePayload();           ///< 长度字段模式帧完成处理
    bool handleCrcValidation();              ///< CRC校验验证
    void handleChecksumVerifying(unsigned char byte); ///< 校验字段接收状态
    void handleFooterMatching(unsigned char byte); ///< 帧尾匹配状态

    // ---- 辅助方法(实现在FrameParserHelpers.cpp) ----
    QVariantMap extractFields(const QByteArray& frameData) const; ///< 从帧数据提取字段值
    bool verifyChecksum(const QByteArray& frameData) const; ///< 验证帧校验和
    int parseLengthField(const QByteArray& frameData) const; ///< 解析长度字段值
    QByteArray computeChecksum(const QByteArray& data) const; ///< 计算校验和

    // ---- 状态机基础设施 ----
    bool checkTimeout();                     ///< 检查帧接收是否超时
    void completeFrame();                    ///< 完成一帧解析(提取字段+发射信号)
    void stopTimeoutTimer();                 ///< 停止超时定时器
    void startTimeoutTimer();                ///< 启动超时定时器
    void resetIntermediateState();           ///< 重置中间状态回到Idle

    // ---- 成员变量 ----
    State m_state = State::Idle;            ///< 当前解析状态
    FrameDefinition m_def;                  ///< 当前帧格式定义
    QByteArray m_buffer;                    ///< 当前帧缓冲区
    int m_headerMatchPos = 0;               ///< 帧头匹配进度
    int m_footerMatchPos = 0;               ///< 帧尾匹配进度
    int m_expectedPayload = 0;              ///< 期望的有效数据长度
    quint64 m_frameCount = 0;               ///< 成功解析帧计数(兼容旧接口)
    quint64 m_errorCount = 0;               ///< 错误帧计数(兼容旧接口)

    quint64 m_totalFramesParsed = 0;        ///< 成功解析帧总数
    quint64 m_totalBytesInput = 0;          ///< 累计输入字节总数
    quint64 m_totalChecksumErrors = 0;      ///< 校验和错误总数
    quint64 m_totalOverflows = 0;           ///< 累计溢出次数
    quint64 m_totalParseErrors = 0;         ///< 累计解析错误次数(含格式/长度/帧尾/超时)
    quint64 m_totalBytesParsed = 0;         ///< 成功解析帧中的有效数据字节总数
    quint64 m_totalSyncLost = 0;            ///< 累计同步丢失次数(帧头匹配失败导致缓冲区清空)
    quint64 m_totalFramesBuilt = 0;         ///< 累计帧构建完成次数(completeFrame调用)
    quint64 m_totalValidationErrors = 0;    ///< 累计校验验证失败次数(CRC/校验和不匹配)
    quint64 m_totalAutoDetectCalls = 0;     ///< 累计自动检测调用次数(为ProtocolBridgeManager预留)
    quint64 m_totalMalformedFrames = 0;     ///< 累计畸形帧次数(帧头/帧尾/长度异常)
    double m_avgFrameSize = 0.0;            ///< 平均帧大小(字节)

    int m_maxFrameLength = kDefaultMaxFrameLength; ///< 帧长度上限（默认1024字节）
    int m_frameTimeoutMs = 500;             ///< 帧超时阈值（毫秒），0=禁用
    QElapsedTimer m_frameTimer;             ///< 帧接收计时器
    QTimer* m_timeoutCheckTimer = nullptr;  ///< 独立的超时检查定时器

    static constexpr int kMaxFrameSize = 4096;          ///< 硬性上限
    static constexpr int kDefaultMaxFrameLength = 1024;  ///< 默认帧长度上限
};

#endif // FRAMEPARSER_H
