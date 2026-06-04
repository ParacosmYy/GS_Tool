/**
 * @file PacketReassembler.h
 * @brief 数据包重组引擎 -- 碎片化协议帧重组
 *
 * 从连续字节流中自动检测帧边界并重组完整数据包。
 * 支持4种重组模式: 固定头/起止标记/长度字段/超时。
 * 协作: SerialConnection(数据源)/FrameParser(下游消费)/ProtocolEngine(协议层)
 */

#ifndef PACKETREASSEMBLER_H
#define PACKETREASSEMBLER_H

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>
#include <QTimer>
#include <QPair>

/**
 * @brief 数据包重组引擎
 *
 * 嵌入式协议经常将数据分片在多帧中传输，本引擎负责:
 *   1. 检测帧边界（根据配置的分隔模式）
 *   2. 缓冲并累积分片数据
 *   3. 重组为完整数据包后发射信号
 *   4. 记录分片数/组装时间/超时丢包等统计
 */
class PacketReassembler : public QObject {
    Q_OBJECT

public:
    /** @brief 分隔模式 */
    enum class DelimiterMode {
        FixedHeader,     ///< 固定帧头模式: 识别帧头+固定长度
        StartEndMarkers, ///< 起止标记模式: 在startMarker和endMarker之间
        LengthField,     ///< 长度字段模式: 从指定偏移读取长度
        Timeout,         ///< 超时模式: 收集数据直到超时
        Manual           ///< 手动模式: 外部调用者控制边界
    };
    Q_ENUM(DelimiterMode)

    /** @brief 重组配置 */
    struct ReassemblyConfig {
        DelimiterMode mode = DelimiterMode::Timeout; ///< 分隔模式
        QByteArray startMarker;          ///< 起始标记(StartEndMarkers/FixedHeader模式)
        QByteArray endMarker;            ///< 结束标记(StartEndMarkers模式)
        int lengthFieldOffset = 0;       ///< 长度字段偏移(LengthField模式)
        int lengthFieldSize = 2;         ///< 长度字段大小(1/2/4字节, LengthField模式)
        int maxPacketSize = 4096;        ///< 最大包大小(防内存溢出)
        int timeoutMs = 500;             ///< 分片超时阈值(ms, 0=禁用)
        int fixedPacketLength = 0;       ///< 固定包长度(FixedHeader模式, 0=使用帧头中的长度字段)
        bool lengthIncludesHeader = false; ///< 长度字段是否包含帧头自身
        int headerSize = 0;              ///< 帧头大小(FixedHeader模式)
    };

    /** @brief 重组完成的数据包 */
    struct ReassembledPacket {
        QByteArray data;            ///< 完整数据包数据
        int fragmentCount = 0;      ///< 分片数量
        qint64 firstFragmentTime = 0; ///< 首个分片时间(epoch ms)
        qint64 lastFragmentTime = 0;  ///< 末尾分片时间(epoch ms)
        qint64 assemblyTimeMs = 0;   ///< 组装耗时(ms)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFragmentsReceived = 0;  ///< 累计接收分片数
        quint64 totalPacketsReassembled = 0; ///< 累计重组完成包数
        quint64 totalPartialPackets = 0;     ///< 当前缓冲中的未完成包数
        quint64 totalTimeoutDrops = 0;       ///< 超时丢弃次数
        quint64 totalOversizeDrops = 0;      ///< 超大包丢弃次数
        quint64 totalBytesProcessed = 0;     ///< 累计处理字节数
        double avgFragmentsPerPacket = 0.0;  ///< 平均每包分片数
        double avgAssemblyTimeMs = 0.0;      ///< 平均组装耗时(ms)
        int peakFragmentCount = 0;           ///< 单包最大分片数
        int currentBufferSize = 0;           ///< 当前缓冲区大小(字节)
    };

    //-- 构造/析构 --//
    explicit PacketReassembler(QObject* parent = nullptr); ///< 构造(初始化默认配置)
    ~PacketReassembler() override;                         ///< 析构(停止定时器)

    //-- 配置接口 --//
    void setConfig(const ReassemblyConfig& config); ///< 设置重组配置(验证后重置缓冲区)
    const ReassemblyConfig& config() const;          ///< 获取当前配置

    //-- 数据输入 --//
    void feedData(const QByteArray& data); ///< 喂入数据(追加到缓冲区, 触发模式处理)

    //-- 状态控制 --//
    void reset();                  ///< 重置重组状态(清空缓冲区, 不清零统计)
    bool isReassembling() const;   ///< 是否正在重组(缓冲区非空)
    int bufferedBytes() const;     ///< 缓冲区中的字节数

    //-- 统计接口 --//
    const Stats& stats() const;    ///< 获取运行统计
    void resetStatistics();        ///< 重置所有统计计数器

signals:
    /** @brief 数据包重组完成 @param packet 重组完成的数据包 */
    void packetReassembled(const PacketReassembler::ReassembledPacket& packet);
    /** @brief 接收到分片数据 @param size 分片字节大小 */
    void fragmentReceived(int size);
    /** @brief 重组超时,缓冲数据被丢弃 @param lostBytes 丢弃的字节数 */
    void reassemblyTimeout(int lostBytes);
    /** @brief 数据包被丢弃 @param reason 丢弃原因(1=超时 2=超大 3=标记错误) */
    void packetDropped(int reason);

private:
    //-- 模式处理方法 --//
    void processFixedHeader();       ///< 固定帧头模式处理
    void processStartEndMarkers();   ///< 起止标记模式处理
    void processLengthField();       ///< 长度字段模式处理
    void processTimeout();           ///< 超时模式处理(启动/重启定时器)

    //-- 辅助方法 --//
    void emitPacket(const QByteArray& packetData); ///< 创建并发送重组包
    bool checkTimeout();                ///< 检查是否超时
    void dropBuffer(int reason);        ///< 丢弃缓冲区数据并发射信号
    int readLengthField(const QByteArray& data, int offset, int fieldSize) const; ///< 从数据中读取长度字段

    //-- 成员变量 --//
    ReassemblyConfig m_config;          ///< 重组配置
    QByteArray m_buffer;                ///< 数据缓冲区
    int m_expectedLength = -1;          ///< 期望的包长度(-1=未知)
    qint64 m_firstFragmentTime = 0;     ///< 首个分片接收时间(epoch ms)
    int m_currentFragmentCount = 0;     ///< 当前包的分片计数
    QElapsedTimer m_assemblyTimer;      ///< 组装计时器
    QTimer* m_timeoutTimer = nullptr;   ///< 超时检测定时器
    Stats m_stats;                      ///< 运行统计

    //-- 累计统计辅助 --//
    quint64 m_sumFragmentsPerPacket = 0; ///< 分片数总和(用于计算平均值)
    quint64 m_sumAssemblyTimeMs = 0;     ///< 组装时间总和(用于计算平均值)

    static constexpr int kDefaultMaxPacketSize = 4096; ///< 默认最大包大小
    static constexpr int kHardMaxPacketSize = 65536;   ///< 硬性上限(防内存溢出)
};

#endif // PACKETREASSEMBLER_H
