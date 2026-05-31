#ifndef SENDHISTORY_H
#define SENDHISTORY_H

#include <QObject>
#include <QStringList>
#include <QDateTime>
#include <QList>

// 发送历史记录条目
struct SendEntry {
    QString text;       // 命令内容
    bool isHex;         // 是否HEX格式
    QDateTime time;     // 发送时间
};

// 发送历史管理器 - 记录和查询发送历史
// 职责：
//   1. 维护最近N条发送命令的去重记录
//   2. 提供关键词搜索能力（大小写不敏感）
//   3. 为发送框下拉列表提供文本数据
class SendHistory : public QObject {
    Q_OBJECT
public:
    explicit SendHistory(QObject* parent = nullptr);

    // 添加一条发送记录
    // 与最后一条内容相同时不添加（防止连续重复刷屏）
    void addEntry(const QString& text, bool isHex);

    // 获取最近的N条记录（纯文本列表，用于下拉框自动补全）
    QStringList recentTexts(int count = 50) const;

    // 获取所有记录
    QList<SendEntry> entries() const;

    // 搜索历史（按关键词过滤，大小写不敏感）
    QList<SendEntry> search(const QString& keyword) const;

    // 清空历史
    void clear();

    // 设置最大记录数（默认50）
    void setMaxEntries(int max);

signals:
    // 历史记录发生变更时发出（添加/清空）
    void historyChanged();

private:
    QList<SendEntry> m_entries;     // 历史记录列表，按时间顺序排列
    int m_maxEntries = 50;          // 最大记录条数
};

#endif // SENDHISTORY_H
