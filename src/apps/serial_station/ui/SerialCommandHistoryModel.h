#ifndef SERIAL_COMMAND_HISTORY_MODEL_H
#define SERIAL_COMMAND_HISTORY_MODEL_H

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

namespace serial_station {

/**
 * @brief Serial Station 最近命令历史条目。
 */
struct SerialCommandHistoryEntry {
    QString command;
    QString mode;
    int useCount = 0;
    QDateTime lastUsed;
};

/**
 * @brief Serial Station 命令面板最近命令模型。
 *
 * 该模型只维护 UI 层字符串历史，不拼接协议帧，不访问串口 core。
 */
class SerialCommandHistoryModel {
public:
    explicit SerialCommandHistoryModel(int maxItems = 8);

    /**
     * @brief 记录一条最近命令。
     * @param command 命令文本
     * @param mode 发送模式
     * @return true 表示历史发生变化
     */
    bool recordCommand(const QString& command, const QString& mode);

    /**
     * @brief 清空最近命令。
     */
    void clear();

    /**
     * @brief 设置最大保留条数。
     * @param maxItems 最大条数，小于 1 时按 1 处理
     */
    void setMaxItems(int maxItems);

    /**
     * @brief 最大保留条数。
     */
    int maxItems() const;

    /**
     * @brief 当前历史条数。
     */
    int count() const;

    /**
     * @brief 是否为空。
     */
    bool isEmpty() const;

    /**
     * @brief 历史条目快照，按最新优先排序。
     */
    QVector<SerialCommandHistoryEntry> entries() const;

    /**
     * @brief 命令文本快照，按最新优先排序。
     */
    QStringList commands() const;

    /**
     * @brief 指定位置的命令文本。
     */
    QString commandAt(int index) const;

    /**
     * @brief 指定位置的发送模式。
     */
    QString modeAt(int index) const;

    /**
     * @brief 指定位置的展示文本。
     */
    QString displayTextAt(int index) const;

    /**
     * @brief 查找命令和模式组合。
     */
    int indexOf(const QString& command, const QString& mode) const;

    /**
     * @brief 是否包含命令和模式组合。
     */
    bool contains(const QString& command, const QString& mode) const;

private:
    static QString normalizedCommand(const QString& command);
    static QString normalizedMode(const QString& mode);
    static QString displayMode(const QString& mode);
    void trimToLimit();

    QVector<SerialCommandHistoryEntry> m_entries;
    int m_maxItems = 8;
};

} // namespace serial_station

#endif // SERIAL_COMMAND_HISTORY_MODEL_H
