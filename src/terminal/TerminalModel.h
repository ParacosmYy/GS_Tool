#ifndef TERMINALMODEL_H
#define TERMINALMODEL_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QMutex>
#include <QDateTime>
#include "terminal/TerminalTypes.h"

// 终端数据模型 - 管理接收/发送的数据缓冲区
// 线程安全，可从任意线程添加数据
// 内部使用环形缓冲区，避免 removeFirst() 的 O(n) 开销
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

signals:
    // 新数据到达，需要重新渲染
    void dataAppended(int firstNewLine, int count);

    // 数据被清空
    void dataCleared();

private:
    // 将逻辑索引转换为环形缓冲区的物理索引
    int physicalIndex(int logicalIndex) const;

    // 向环形缓冲区追加一行，自动处理容量和覆盖
    void appendLine(TerminalLine&& line);

    QVector<TerminalLine> m_buffer;    // 环形缓冲区，容量 = m_maxLines
    int m_head = 0;                    // 环形缓冲区头指针（最旧数据位置）
    int m_count = 0;                   // 当前有效数据行数
    mutable QMutex m_mutex;            // 线程安全锁
    int m_maxLines = 50000;            // 最大行数限制
    quint64 m_rxBytes = 0;             // 接收总字节数
    quint64 m_txBytes = 0;             // 发送总字节数
};

#endif // TERMINALMODEL_H
