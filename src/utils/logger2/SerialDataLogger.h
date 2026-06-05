/**
 * @file SerialDataLogger.h
 * @brief 高级串口数据日志记录器 -- 多格式输出/日志轮转/自动压缩
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 6种格式(Raw/Hex/ASCII/CSV/JSON/PCAP)、按大小/时间/数量轮转、gzip 压缩旧日志。
 * 所属层级: 数据层(纯文件 I/O)，协作: 上层 Controller 调用 logData()
 */
#ifndef SERIALDATALOGGER_H
#define SERIALDATALOGGER_H

#include <QDir>
#include <QFile>
#include <QObject>
#include <QTimer>

class SerialDataLogger : public QObject {
    Q_OBJECT

public:
    /** @brief 日志输出格式 */
    enum class LogFormat {
        Raw = 0, Hex = 1, Ascii = 2, Csv = 3, Json = 4, Pcap = 5
    };
    Q_ENUM(LogFormat)

    /** @brief 时间戳格式 */
    enum class TimestampFormat {
        None = 0, Epoch = 1, ISO8601 = 2, Relative = 3, Custom = 4
    };
    Q_ENUM(TimestampFormat)

    /** @brief 轮转时间周期 */
    enum class RotationPeriod { None = 0, Hourly = 1, Daily = 2 };
    Q_ENUM(RotationPeriod)

    /** @brief 全局统计摘要 */
    struct Stats {
        quint64 totalBytesLogged  = 0; ///< 累计写入字节
        quint64 totalRecords      = 0; ///< 累计记录条数
        quint64 totalFilesCreated = 0; ///< 累计创建文件
        quint64 totalRotations    = 0; ///< 累计轮转次数
        quint64 totalCompressions = 0; ///< 累计压缩次数
        quint64 bufferSize        = 0; ///< 当前缓冲区字节
        quint64 currentFileSize   = 0; ///< 当前文件大小
    };

    explicit SerialDataLogger(QObject* parent = nullptr);
    ~SerialDataLogger() override;
    SerialDataLogger(const SerialDataLogger&) = delete;
    SerialDataLogger& operator=(const SerialDataLogger&) = delete;

    // ── 日志控制 ──

    /** @brief 开始记录。若已在记录则先停止。pattern 支持 {date}/{time}/{index} */
    bool startLogging(const QString& dirPath,
                      const QString& pattern = QStringLiteral("serial_{date}_{time}"),
                      LogFormat format = LogFormat::Csv);
    void stopLogging();   ///< 停止记录，刷新并关闭文件
    void pauseLogging();  ///< 暂停(文件保持打开)
    void resumeLogging(); ///< 恢复记录
    bool isLogging() const;
    bool isPaused() const;

    // ── 数据写入 ──

    /** @brief 记录一条数据。direction 为 "RX"/"TX"。缓冲区满则自动 flush */
    void logData(const QByteArray& data,
                 const QString& direction = QStringLiteral("RX"));
    void flush(); ///< 手动刷新缓冲区到磁盘

    // ── 配置 ──

    void setMaxFileSize(quint64 bytes);       ///< 最大文件大小(0=不限)
    quint64 maxFileSize() const;
    void setMaxFiles(int count);              ///< 最大文件数(0=不限)
    void setRotationPeriod(RotationPeriod p); ///< 轮转时间周期
    void setTimestampFormat(TimestampFormat f);///< 时间戳格式
    void setCustomTimestampFormat(const QString& fmt); ///< 自定义时间戳(Qt 日期格式)
    void setBufferSize(quint64 bytes);        ///< 缓冲区大小上限

    // ── 查询 ──

    QString getCurrentLogFile() const;        ///< 当前日志文件路径
    QStringList getLogFiles() const;          ///< 所有日志文件(时间升序)
    /** @brief 将当前日志导出为其他格式 */
    bool exportLog(const QString& outputPath, LogFormat format) const;

    // ── 统计 ──
    Stats stats() const;
    void resetStatistics();

signals:
    void logStarted();                        ///< 日志已启动
    void logStopped();                        ///< 日志已停止
    void logRotated(const QString& newPath);  ///< 日志轮转
    void error(const QString& message);       ///< 错误(已 tr() 包裹)

private slots:
    void onFlushTimer();     ///< 定时 flush 回调
    void onRotationCheck();  ///< 定时轮转检查回调

private:
    // 内部方法
    QString resolvePattern(const QString& pattern) const;
    bool openLogFile(const QString& filePath);
    void closeLogFile();
    void performRotation();
    void compressOldFiles();
    void enforceMaxFiles();
    QString formatTimestamp() const;
    QByteArray formatRecord(const QByteArray& data, const QString& dir) const;
    QByteArray buildPcapGlobalHeader() const;
    QByteArray buildPcapPacketHeader(quint32 len) const;
    void writeToFile(const QByteArray& bytes);

    // 状态
    bool m_logging = false;
    bool m_paused  = false;

    // 文件
    QFile* m_file          = nullptr;
    QString m_logDirPath;
    QString m_filePattern;
    QString m_currentFilePath;
    LogFormat m_format     = LogFormat::Csv;
    QDir m_logDir;

    // 配置
    quint64 m_maxFileSize  = 10 * 1024 * 1024;
    int m_maxFiles         = 0;
    RotationPeriod m_rotationPeriod   = RotationPeriod::None;
    TimestampFormat m_timestampFormat = TimestampFormat::ISO8601;
    QString m_customTimestampFormat   = QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz");
    quint64 m_bufferLimit  = 65536;

    // 缓冲区与定时器
    QByteArray m_buffer;
    QTimer* m_flushTimer    = nullptr;
    QTimer* m_rotationTimer = nullptr;

    // 时间基准
    qint64 m_startEpoch       = 0;
    qint64 m_rotationBaseEpoch = 0;

    // 统计
    quint64 m_totalBytesLogged  = 0;
    quint64 m_totalRecords      = 0;
    quint64 m_totalFilesCreated = 0;
    quint64 m_totalRotations    = 0;
    quint64 m_totalCompressions = 0;
    quint64 m_currentFileSize   = 0;

    // PCAP 常量
    static constexpr quint32 kPcapMagic   = 0xa1b2c3d4;
    static constexpr quint16 kPcapMajor   = 2;
    static constexpr quint16 kPcapMinor   = 4;
};

#endif // SERIALDATALOGGER_H
