/**
 * @file ClipboardManager.h
 * @brief 剪贴板管理器 - 管理应用内剪贴板历史和系统剪贴板交互
 *
 * 职责:
 *   1. 维护应用内部的剪贴板历史记录（文本和二进制）
 *   2. 与系统剪贴板双向同步
 *   3. 支持历史条目的固定（pin）和取消固定
 *   4. 历史条目数量上限可配置
 */

#pragma once
#include <QObject>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QClipboard>

/**
 * @brief 剪贴板管理器 - 管理应用内剪贴板历史和系统剪贴板交互
 *
 * 提供文本/二进制数据的推送、历史查询、条目固定、
 * 以及与系统剪贴板的双向同步能力。
 */
class ClipboardManager : public QObject {
    Q_OBJECT
public:
    /** @brief 剪贴板条目结构体 */
    struct ClipEntry {
        QString text;           ///< 文本内容
        QByteArray binary;      ///< 二进制数据
        qint64 timestamp;       ///< 时间戳（毫秒级）
        QString format;         ///< 数据格式标识（如 "hex"、"utf-8"）
    };

    /**
     * @brief 构造剪贴板管理器
     * @param parent 父对象
     */
    explicit ClipboardManager(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~ClipboardManager() override;

    /**
     * @brief 推送文本到内部历史
     * @param text 文本内容
     */
    void pushText(const QString &text);

    /**
     * @brief 推送二进制数据到内部历史
     * @param data 二进制数据
     * @param format 数据格式标识，默认 "hex"
     */
    void pushBinary(const QByteArray &data, const QString &format = "hex");

    /**
     * @brief 将文本写入系统剪贴板
     * @param text 要写入的文本
     */
    void pushToSystemClipboard(const QString &text);

    /**
     * @brief 获取系统剪贴板当前文本
     * @return 系统剪贴板文本内容，无内容时返回空字符串
     */
    QString systemClipboardText() const;

    /**
     * @brief 获取指定索引的历史条目
     * @param index 条目索引
     * @return 历史条目结构体
     */
    ClipEntry entry(int index) const;

    /**
     * @brief 获取完整历史列表
     * @return 所有历史条目的列表
     */
    QList<ClipEntry> history() const;

    /**
     * @brief 获取历史条目数量
     * @return 当前历史条目总数
     */
    int historySize() const;

    /**
     * @brief 设置历史条目数量上限
     * @param max 最大条目数
     */
    void setMaxHistory(int max);

    /** @brief 清空所有历史条目 */
    void clearHistory();

    /**
     * @brief 固定指定条目，防止被溢出淘汰
     * @param index 条目索引
     */
    void pinEntry(int index);

    /**
     * @brief 取消固定指定条目
     * @param index 条目索引
     */
    void unpinEntry(int index);

signals:
    /** @brief 新条目添加到历史时发射 @param entry 新增的条目 */
    void entryAdded(const ClipEntry &entry);

    /** @brief 系统剪贴板内容变更时发射 */
    void clipboardChanged();

    /** @brief 获取累计推送文本次数 @return 计数 */
    quint64 totalTextPushes() const;
    /** @brief 获取累计推送二进制次数 @return 计数 */
    quint64 totalBinaryPushes() const;
    /** @brief 获取累计系统剪贴板写入次数 @return 计数 */
    quint64 totalSystemWrites() const;
    /** @brief 获取累计系统剪贴板变更事件次数 @return 计数 */
    quint64 totalClipboardChanges() const;
    /** @brief 获取累计固定条目次数 @return 计数 */
    quint64 totalPins() const;
    /** @brief 获取累计取消固定条目次数 @return 计数 */
    quint64 totalUnpins() const { return m_totalUnpins; }
    /** @brief 获取累计清空历史次数 @return 计数 */
    quint64 totalClears() const { return m_totalClears; }
    /** @brief 获取累计溢出淘汰条目数 @return 计数 */
    quint64 totalEvictions() const { return m_totalEvictions; }
    /** @brief 重置所有剪贴板管理器统计计数器 */
    void resetClipboardStatistics();

private:
    /** @brief 系统剪贴板变更回调 */
    void onClipboardChanged();

    QList<ClipEntry> m_history;     ///< 历史条目列表
    int m_maxHistory = 50;          ///< 最大历史条目数
    QClipboard *m_clipboard = nullptr; ///< 系统剪贴板实例

    quint64 m_totalTextPushes = 0;        ///< 累计推送文本次数
    quint64 m_totalBinaryPushes = 0;      ///< 累计推送二进制次数
    quint64 m_totalSystemWrites = 0;      ///< 累计系统剪贴板写入次数
    quint64 m_totalClipboardChanges = 0;  ///< 累计系统剪贴板变更事件次数
    quint64 m_totalPins = 0;              ///< 累计固定条目次数
    quint64 m_totalUnpins = 0;            ///< 累计取消固定条目次数
    quint64 m_totalClears = 0;            ///< 累计清空历史次数
    quint64 m_totalEvictions = 0;         ///< 累计溢出淘汰条目数
};
