/**
 * @file SerialDataLogger.h
 * @brief 高级串口数据日志记录器 -- 支持多格式输出、日志轮转和自动压缩
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供串口通信数据的高级日志记录能力：
 * - 6种输出格式(Raw/Hex/ASCII/CSV/JSON/PCAP)
 * - 按大小/时间/数量自动轮转
 * - 自动 gzip 压缩旧日志
 * - 可配置时间戳格式(无/Epoch/ISO8601/相对/自定义)
 * - 格式转换导出
 */

#ifndef SERIALDATALOGGER_H
#define SERIALDATALOGGER_H

#include <QDir>
#include <QFile>
#include <QMutex>
#include <QObject>
#include <QTimer>

/**
 * @class SerialDataLogger
 * @brief 高级串口数据日志记录器
 *
 * 核心设计：
 * - 数据写入支持6种格式，可运行时切换
 * - 日志轮转策略: 大小上限 / 时间周期(小时/天) / 文件数量上限
 * - 轮转后的旧文件自动 gzip 压缩
 * - 缓冲区批量写入，定时或手动 flush
 * - 格式转换: exportLog() 可将已有日志转为其他格式输出
 *
 * 所属层级: 数据层(纯文件 I/O，不涉及 UI)
 * 协作: 上层 Controller 调用 logData() 记录数据
 */
class SerialDataLogger : public QObject {
    Q_OBJECT

public:
    /** @brief 日志输出格式 */
    enum class LogFormat {
        Raw  = 0,  ///< 原始二进制
        Hex  = 1,  ///< Hex dump(十六进制文本)
        Ascii = 2, ///< ASCII 文本
        Csv  = 3,  ///< CSV(带时间戳)
        Json = 4,  ///< JSON 数组(每条记录一个对象)
        Pcap = 5   ///< PCAP 捕获文件
    };
    Q_ENUM(LogFormat)

    /** @brief 时间戳格式 */
    enum class TimestampFormat {
        None     = 0, ///< 不附加时间戳
        Epoch    = 1, ///< Unix 毫秒时间戳
        ISO8601  = 2, ///< ISO 8601 字符串
        Relative = 3, ///< 相对于日志启动的毫秒数
        Custom   = 4  ///< 用户自定义 Qt 日期格式
    };
    Q_ENUM(TimestampFormat)

    /** @brief 轮转时间周期 */
    enum class RotationPeriod {
        None   = 0, ///< 不按时间轮转
        Hourly = 1, ///< 每小时
        Daily  = 2  ///< 每天
    };
    Q_ENUM(RotationPeriod)

    /** @brief 全局统计摘要 */
    struct Stats {
        quint64 totalBytesLogged    = 0;  ///< 累计写入的字节总数
        quint64 totalRecords        = 0;  ///< 累计记录条数
        quint64 totalFilesCreated   = 0;  ///< 累计创建的文件总数
        quint64 totalRotations      = 0;  ///< 累计轮转次数
        quint64 totalCompressions   = 0;  ///< 累计压缩次数
        quint64 bufferSize          = 0;  ///< 当前缓冲区中的字节数
        quint64 currentFileSize     = 0;  ///< 当前日志文件大小(字节)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit SerialDataLogger(QObject* parent = nullptr);

    /** @brief 析构函数，停止日志并刷新缓冲区 */
    ~SerialDataLogger() override;

    SerialDataLogger(const SerialDataLogger&) = delete;
    SerialDataLogger& operator=(const SerialDataLogger&) = delete;

    // ── 日志控制 ──

    /**
     * @brief 开始日志记录
     *
     * 创建日志文件并开始接受数据。若已在记录则先停止再重启。
     *
     * @param dirPath 日志目录路径
     * @param pattern 文件名模式(支持 {date}/{time}/{index} 占位符)
     * @param format 输出格式
     * @return true 启动成功
     */
    bool startLogging(const QString& dirPath,
                      const QString& pattern = QStringLiteral("serial_{date}_{time}"),
                      LogFormat format = LogFormat::Csv);

    /** @brief 停止日志记录，刷新并关闭文件 */
    void stopLogging();

    /** @brief 暂停记录(文件保持打开) */
    void pauseLogging();

    /** @brief 恢复记录 */
    void resumeLogging();

    /** @brief 是否正在记录 */
    bool isLogging() const;

    /** @brief 是否暂停 */
    bool isPaused() const;

    // ── 数据写入 ──

    /**
     * @brief 记录一条串口数据
     * @param data 数据内容
     * @param direction 数据方向("RX" 或 "TX")
     *
     * 若缓冲区已满则自动 flush。暂停状态下数据被丢弃。
     */
    void logData(const QByteArray& data, const QString& direction = QStringLiteral("RX"));

    /** @brief 手动刷新缓冲区到磁盘 */
    void flush();

    // ── 配置接口 ──

    /** @brief 设置最大文件大小(字节)，超过后自动轮转。0 = 不限制 */
    void setMaxFileSize(quint64 bytes);

    /** @brief 获取最大文件大小 */
    quint64 maxFileSize() const;

    /** @brief 设置最大日志文件数量，超出后删除最旧文件。0 = 不限制 */
    void setMaxFiles(int count);

    /** @brief 设置轮转时间周期 */
    void setRotationPeriod(RotationPeriod period);

    /** @brief 设置时间戳格式 */
    void setTimestampFormat(TimestampFormat format);

    /** @brief 设置自定义时间戳格式字符串(Qt 日期格式) */
    void setCustomTimestampFormat(const QString& fmt);

    /** @brief 设置缓冲区大小上限(字节) */
    void setBufferSize(quint64 bytes);

    // ── 查询接口 ──

    /** @brief 获取当前日志文件路径 */
    QString getCurrentLogFile() const;

    /** @brief 获取所有日志文件路径列表(按时间升序) */
    QStringList getLogFiles() const;

    /**
     * @brief 导出日志到指定格式
     * @param outputPath 输出文件路径
     * @param format 目标格式
     * @return true 导出成功
     *
     * 读取当前日志文件并以新格式重新写入 outputPath。
     */
    bool exportLog(const QString& outputPath, LogFormat format) const;

    // ── 统计 ──

    /** @brief 获取统计快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 日志记录已启动 */
    void logStarted();

    /** @brief 日志记录已停止 */
    void logStopped();

    /**
     * @brief 日志文件发生轮转
     * @param newFilePath 轮转后的新文件路径
     */
    void logRotated(const QString& newFilePath);

    /**
     * @brief 错误信号
     * @param message 错误描述(已用 tr() 包裹)
     */
    void error(const QString& message);

private slots:
    /** @brief 定时 flush 回调 */
    void onFlushTimer();

    /** @brief 定时轮转检查回调 */
    void onRotationCheck();

private:
    // ── 内部方法 ──
    QString resolvePattern(const QString& pattern) const;  ///< 解析文件名占位符
    bool openLogFile(const QString& filePath);              ///< 打开日志文件(含格式头)
    void closeLogFile();                                    ///< 关闭日志文件(含格式尾)
    void performRotation();                                 ///< 执行轮转
    void compressOldFiles();                                ///< gzip 压缩旧日志
    void enforceMaxFiles();                                 ///< 删除超限的旧文件
    QString formatTimestamp() const;                        ///< 按配置格式化时间戳
    QByteArray formatRecord(const QByteArray& data,
                            const QString& direction) const; ///< 将数据格式化为日志记录
    QByteArray buildPcapGlobalHeader() const;               ///< 构建 PCAP 全局头
    QByteArray buildPcapPacketHeader(quint32 len) const;    ///< 构建 PCAP 包头
    void writeToFile(const QByteArray& bytes);              ///< 写入文件并更新统计

    // ── 状态 ──
    bool m_logging = false;   ///< 是否正在记录
    bool m_paused  = false;   ///< 是否暂停

    // ── 文件 ──
    QFile* m_file = nullptr;               ///< 当前日志文件
    QString m_logDirPath;                  ///< 日志目录
    QString m_filePattern;                 ///< 文件名模式
    QString m_currentFilePath;             ///< 当前文件完整路径
    LogFormat m_format = LogFormat::Csv;   ///< 输出格式
    QDir m_logDir;                         ///< 日志目录操作对象

    // ── 配置 ──
    quint64 m_maxFileSize   = 10 * 1024 * 1024; ///< 最大文件大小(默认 10MB)
    int m_maxFiles          = 0;                 ///< 最大文件数(0 = 无限)
    RotationPeriod m_rotationPeriod = RotationPeriod::None; ///< 轮转周期
    TimestampFormat m_timestampFormat = TimestampFormat::ISO8601; ///< 时间戳格式
    QString m_customTimestampFormat = QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"); ///< 自定义时间戳
    quint64 m_bufferLimit   = 65536;      ///< 缓冲区大小上限(64KB)

    // ── 缓冲区 ──
    QByteArray m_buffer;                   ///< 写入缓冲区

    // ── 定时器 ──
    QTimer* m_flushTimer     = nullptr;    ///< 定时 flush 定时器(1秒)
    QTimer* m_rotationTimer  = nullptr;    ///< 轮转检查定时器(30秒)

    // ── 时间基准 ──
    qint64 m_startEpoch = 0;              ///< 日志启动时的 epoch 毫秒
    qint64 m_rotationBaseEpoch = 0;       ///< 上次轮转检查基准时间

    // ── 统计计数器 ──
    quint64 m_totalBytesLogged  = 0;      ///< 累计写入字节
    quint64 m_totalRecords      = 0;      ///< 累计记录条数
    quint64 m_totalFilesCreated = 0;      ///< 累计创建文件数
    quint64 m_totalRotations    = 0;      ///< 累计轮转次数
    quint64 m_totalCompressions = 0;      ///< 累计压缩次数
    quint64 m_currentFileSize   = 0;      ///< 当前文件大小

    // ── PCAP 元信息 ──
    static constexpr quint32 kPcapMagicNumber = 0xa1b2c3d4; ///< PCAP 魔数
    static constexpr quint16 kPcapVersionMajor = 2;          ///< PCAP 主版本号
    static constexpr quint16 kPcapVersionMinor = 4;          ///< PCAP 次版本号
};

#endif // SERIALDATALOGGER_H
