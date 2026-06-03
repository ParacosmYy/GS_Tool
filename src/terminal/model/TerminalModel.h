/**
 * @file TerminalModel.h
 * @brief 终端数据模型 — 管理终端显示数据的环形缓冲区接口
 */
#ifndef TERMINALMODEL_H
#define TERMINALMODEL_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QMutex>
#include <QDateTime>
#include "terminal/types/TerminalTypes.h"

/**
 * @brief 终端数据模型 — 管理终端显示数据的线程安全环形缓冲区
 *
 * 存储RX/TX数据行(TerminalLine)，内部使用环形缓冲区避免removeFirst()的O(n)开销。
 * 支持最大行数限制防止内存无限增长，可从任意线程安全添加数据。
 * 属于数据层，不依赖任何表现层组件。
 *
 * 协作关系:
 *   - TerminalWidget: 监听 dataAppended/dataCleared 信号进行渲染
 *   - TerminalController: 读取 rxBytes/txBytes 统计和行数据用于导出
 *   - DataExporter: 通过 lines() 接口批量拉取数据进行文件导出
 */
class TerminalModel : public QObject {
    Q_OBJECT

public:
    explicit TerminalModel(QObject* parent = nullptr);

    // 添加接收到的数据
    void appendReceived(const QByteArray& data);

    // 添加发送的数据
    void appendSent(const QByteArray& data);

    // 获取所有行 (用于导出等场景，返回深拷贝)
    QVector<TerminalLine> lines() const;

    // 获取指定范围的行 (用于导出，返回深拷贝)
    QVector<TerminalLine> lines(int start, int count) const;

    // 获取单行的拷贝 (用于渲染)
    // 返回值而非引用，避免QMutexLocker释放后引用悬空
    // 拷贝成本可接受: QByteArray(隐式共享/COW) + QDateTime + enum
    // 调用者必须保证 index 在 [0, lineCount()) 范围内
    TerminalLine lineAt(int index) const;

    // 获取总行数
    int lineCount() const;

    // 获取接收/发送字节统计
    quint64 rxBytes() const;
    quint64 txBytes() const;

    // 清空所有数据
    void clear();

    // 设置最大行数限制 (防止内存无限增长)
    void setMaxLines(int max);

    // 获取最大行数限制
    int maxLines() const;

    // ---- 统计计数接口 ----
    quint64 totalLinesAdded() const;   ///< 获取累计追加行数
    quint64 totalBytesReceived() const;///< 获取累计接收字节数
    quint64 maxLineLength() const;     ///< 获取历史最长行长度(字节)
    quint64 filterBlockCount() const;  ///< 获取被过滤丢弃的行数
    void resetStats();                 ///< 重置所有统计计数器(保留rxBytes/txBytes)

signals:
    /** @brief 新数据到达通知 @param firstNewLine 新数据起始行号 @param count 新增行数 */
    void dataAppended(int firstNewLine, int count);

    /** @brief 数据已清空 */
    void dataCleared();

private:
    // 将逻辑索引转换为环形缓冲区的物理索引
    int physicalIndex(int logicalIndex) const;

    // 向环形缓冲区追加一行，自动处理容量和覆盖
    void appendLine(TerminalLine&& line);

    QVector<TerminalLine> m_buffer;    ///< 环形缓冲区，容量 = m_maxLines
    int m_head = 0;                    ///< 环形缓冲区头指针（最旧数据位置）
    int m_count = 0;                   ///< 当前有效数据行数
    mutable QMutex m_mutex;            ///< 线程安全锁
    int m_maxLines = 50000;            ///< 最大行数限制
    bool m_settingMaxLines = false;    ///< 重入保护标志，防止 dataCleared 信号回调 setMaxLines
    quint64 m_rxBytes = 0;             ///< 接收总字节数
    quint64 m_txBytes = 0;             ///< 发送总字节数

    // ---- 统计计数 ----
    quint64 m_totalLinesAdded = 0;     ///< 累计追加行数(含被环形缓冲区覆盖的)
    quint64 m_totalBytesReceived = 0;  ///< 累计接收字节数(独立于m_rxBytes)
    quint64 m_maxLineLength = 0;       ///< 历史最长行长度(字节数)
    quint64 m_filterBlockCount = 0;    ///< 被过滤丢弃的行数
};

#endif // TERMINALMODEL_H
