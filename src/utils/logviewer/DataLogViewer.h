/**
 * @file DataLogViewer.h
 * @brief 数据日志查看器 — 加载/搜索/过滤/导出历史数据日志
 *
 * 功能: 加载CSV/BIN/HEX格式的数据日志，支持时间范围过滤、关键字搜索、
 *       数据方向过滤(RX/TX)、正则匹配，结果高亮显示并支持导出。
 *
 * 协作: DataExporter(日志格式兼容) / TerminalWidget(日志加载入口)
 */
#ifndef DATALOGVIEWER_H
#define DATALOGVIEWER_H

#include <QWidget>
#include <QByteArray>
#include <QList>

class QTableWidget;
class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;
class QDateTimeEdit;
class QCheckBox;
class QProgressBar;

/** @brief 单条日志记录 */
struct DataLogEntry {
    qint64 timestamp = 0;       ///< 时间戳(ms)
    int direction = 0;          ///< 方向 (0=RX, 1=TX)
    QByteArray data;            ///< 原始数据
    QString hexString;          ///< 十六进制字符串
    QString asciiString;        ///< ASCII字符串
};

/**
 * @brief 数据日志查看器控件 — 加载、搜索和过滤历史数据
 */
class DataLogViewer : public QWidget {
    Q_OBJECT

public:
    /** @brief 日志格式 */
    enum class LogFormat {
        Csv,        ///< CSV格式
        HexDump,    ///< HEX转储格式
        RawBinary   ///< 原始二进制
    };
    Q_ENUM(LogFormat)

    /** @brief 统计数据 */
    struct Stats {
        quint64 totalEntries = 0;       ///< 累计加载条目数
        quint64 totalBytesLoaded = 0;   ///< 累计加载字节数
        quint64 totalSearches = 0;      ///< 累计搜索次数
        quint64 totalFilters = 0;       ///< 累计过滤次数
        quint64 totalExports = 0;       ///< 累计导出次数
        int     peakEntries = 0;        ///< 峰值条目数
        quint64 matchedResults = 0;     ///< 累计匹配结果数
    };

    /** @brief 构造数据日志查看器 @param parent 父控件 */
    explicit DataLogViewer(QWidget* parent = nullptr);

    /** @brief 从文件加载日志 @param filePath 文件路径 @param format 日志格式 @return 是否成功 */
    bool loadFromFile(const QString& filePath, LogFormat format);

    /** @brief 从内存加载数据 @param entries 日志条目列表 */
    void loadEntries(const QList<DataLogEntry>& entries);

    /** @brief 搜索关键字 @param keyword 关键字 @param useRegex 是否使用正则 */
    void search(const QString& keyword, bool useRegex = false);

    /** @brief 设置方向过滤器 @param direction 方向(-1=全部, 0=RX, 1=TX) */
    void setDirectionFilter(int direction);

    /** @brief 设置时间范围过滤 @param fromMs 起始时间(ms, -1=不限) @param toMs 结束时间(ms, -1=不限) */
    void setTimeFilter(qint64 fromMs, qint64 toMs);

    /** @brief 导出过滤结果 @param filePath 目标文件路径 @return 是否成功 */
    bool exportFiltered(const QString& filePath);

    /** @brief 清除所有数据 */
    void clear();

    /** @brief 获取统计引用 @return Stats常量引用 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 数据加载完成 @param count 加载的条目数 */
    void dataLoaded(int count);
    /** @brief 搜索完成 @param matched 匹配条目数 @param total 总条目数 */
    void searchCompleted(int matched, int total);
    /** @brief 导出完成 @param filePath 文件路径 @param bytes 字节数 */
    void exportCompleted(const QString& filePath, qint64 bytes);

private slots:
    void onSearchClicked();
    void onFilterChanged();
    void onExportClicked();
    void onLoadClicked();

private:
    void setupUI();
    void refreshTable();
    void applyFilters();
    QList<DataLogEntry> parseCsv(const QByteArray& content);
    QList<DataLogEntry> parseHexDump(const QByteArray& content);

    QTableWidget* m_logTable;      ///< 日志显示表格
    QLineEdit* m_searchEdit;       ///< 搜索输入框
    QCheckBox* m_regexCheck;       ///< 正则开关
    QComboBox* m_directionCombo;   ///< 方向过滤
    QDateTimeEdit* m_fromTimeEdit; ///< 起始时间
    QDateTimeEdit* m_toTimeEdit;   ///< 结束时间
    QCheckBox* m_timeFilterCheck;  ///< 时间过滤开关
    QPushButton* m_searchBtn;      ///< 搜索按钮
    QPushButton* m_loadBtn;        ///< 加载按钮
    QPushButton* m_exportBtn;      ///< 导出按钮
    QLabel* m_statusLabel;         ///< 状态标签
    QProgressBar* m_progressBar;   ///< 进度条

    QList<DataLogEntry> m_allEntries;     ///< 全部日志条目
    QList<DataLogEntry> m_filteredEntries;///< 过滤后条目
    QString m_currentKeyword;             ///< 当前搜索关键字
    int m_directionFilter;                ///< 方向过滤(-1=全部)
    qint64 m_timeFrom;                    ///< 时间范围起始
    qint64 m_timeTo;                      ///< 时间范围结束

    Stats m_stats;                        ///< 运行统计
};

#endif // DATALOGVIEWER_H
