#ifndef TERMINALMODEL_H
#define TERMINALMODEL_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QMutex>
#include <QDateTime>
#include "Constants.h"

// 单条终端数据记录
struct TerminalLine {
    QByteArray data;                 // 原始数据
    DataDirection direction;         // 收/发方向
    QDateTime timestamp;             // 时间戳(精确到ms)
};

// 终端数据模型 - 管理接收/发送的数据缓冲区
// 线程安全，可从任意线程添加数据
class TerminalModel : public QObject {
    Q_OBJECT

public:
    explicit TerminalModel(QObject* parent = nullptr);

    // 添加接收到的数据
    void appendReceived(const QByteArray& data);

    // 添加发送的数据
    void appendSent(const QByteArray& data);

    // 获取所有行 (用于渲染)
    QVector<TerminalLine> lines() const;

    // 获取总行数
    int lineCount() const;

    // 获取接收/发送字节统计
    quint64 rxBytes() const;
    quint64 txBytes() const;

    // 清空所有数据
    void clear();

    // 设置最大行数限制 (防止内存无限增长)
    void setMaxLines(int max);

signals:
    // 新数据到达，需要重新渲染
    void dataAppended(int firstNewLine, int count);

    // 数据被清空
    void dataCleared();

private:
    QVector<TerminalLine> m_lines;   // 数据行缓冲区
    mutable QMutex m_mutex;          // 线程安全锁
    int m_maxLines = 50000;          // 最大行数限制
    quint64 m_rxBytes = 0;           // 接收总字节数
    quint64 m_txBytes = 0;           // 发送总字节数
};

#endif // TERMINALMODEL_H
