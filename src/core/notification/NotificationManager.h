/**
 * @file NotificationManager.h
 * @brief 通知管理器，统一管理应用内通知的创建、确认和历史记录
 */
#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QList>
#include <QTimer>

/**
 * @class NotificationManager
 * @brief 通知管理器，支持多优先级通知、确认/消除操作和历史记录裁剪
 */
class NotificationManager : public QObject {
    Q_OBJECT
public:
    /** @brief 通知优先级枚举 */
    enum Priority { Low, Normal, High, Critical };
    Q_ENUM(Priority)

    /**
     * @brief 通知数据结构
     */
    struct Notification {
        QString id;              ///< 通知唯一标识
        QString title;           ///< 通知标题
        QString message;         ///< 通知内容
        Priority priority;       ///< 优先级
        qint64 timestamp;        ///< 创建时间戳
        bool acknowledged;       ///< 是否已确认
    };

    /** @brief 构造函数 @param parent 父对象指针 */
    explicit NotificationManager(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~NotificationManager() override;

    /** @brief 发送一条通知 @param title 标题 @param msg 内容 @param prio 优先级 @return 通知ID */
    QString notify(const QString &title, const QString &msg, Priority prio = Normal);
    /** @brief 确认指定通知 @param id 通知ID */
    void acknowledge(const QString &id);
    /** @brief 消除指定通知 @param id 通知ID */
    void dismiss(const QString &id);
    /** @brief 清除所有通知和历史 */
    void clearAll();
    /** @brief 获取所有未消除的活跃通知 @return 通知列表 */
    QList<Notification> activeNotifications() const;
    /** @brief 获取全部通知(含历史) @return 通知列表 */
    QList<Notification> allNotifications() const;
    /** @brief 获取未确认通知数量 @return 未确认数量 */
    int unreadCount() const;
    /** @brief 设置最大历史记录条数 @param max 最大条数 */
    void setMaxHistory(int max);

signals:
    /** @brief 新通知添加时发射 @param n 通知对象 */
    void notificationAdded(const Notification &n);
    /** @brief 通知被确认时发射 @param id 通知ID */
    void notificationAcknowledged(const QString &id);
    /** @brief 通知被消除时发射 @param id 通知ID */
    void notificationDismissed(const QString &id);
    /** @brief 未确认数量变化时发射 @param count 当前未确认数量 */
    void unreadCountChanged(int count);

private:
    /** @brief 裁剪超出最大条数的历史记录 */
    void pruneHistory();

    QMap<QString, Notification> m_active;   ///< 活跃通知，ID到通知的映射
    QList<Notification> m_history;           ///< 已消除的历史通知
    int m_maxHistory = 100;                  ///< 最大历史条数
    int m_counter = 0;                       ///< 通知ID自增计数器

    // ---- 统计计数器 ----
    quint64 m_totalNotifications = 0;        ///< 总通知创建次数
    quint64 m_totalAcknowledges = 0;         ///< 总确认次数
    quint64 m_totalDismisses = 0;            ///< 总消除次数
    quint64 m_totalClearAlls = 0;            ///< 总全部清除次数

public:
    /** @brief 获取总通知创建次数 @return 累计创建次数 */
    quint64 totalNotifications() const { return m_totalNotifications; }
    /** @brief 获取总确认次数 @return 累计确认次数 */
    quint64 totalAcknowledges() const { return m_totalAcknowledges; }
    /** @brief 获取总消除次数 @return 累计消除次数 */
    quint64 totalDismisses() const { return m_totalDismisses; }
    /** @brief 获取总全部清除次数 @return 累计清除次数 */
    quint64 totalClearAlls() const { return m_totalClearAlls; }
    /** @brief 重置通知统计计数器 */
    void resetNotificationStatistics() { m_totalNotifications = 0; m_totalAcknowledges = 0; m_totalDismisses = 0; m_totalClearAlls = 0; }
};
