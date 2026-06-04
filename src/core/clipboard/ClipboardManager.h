/**
 * @file ClipboardManager.h
 * @brief 剪贴板管理器 - 统一管理应用内剪贴板操作和历史
 * @since score-131
 */
#ifndef CLIPBOARDMANAGER_H
#define CLIPBOARDMANAGER_H
#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>

struct ClipboardEntry {
    QString text;
    QString format;
    qint64 timestampMs;
    QString source;
    bool isValid() const { return !text.isEmpty(); }
};

class ClipboardManager : public QObject {
    Q_OBJECT
public:
    /** @brief 获取剪贴板管理器单例引用 */
    static ClipboardManager &instance();
    /** @brief 复制纯文本到系统剪贴板并记录历史
     *  @param text 待复制的文本内容
     *  @param source 来源标识（可选） */
    void copyText(const QString &text, const QString &source = QString());
    /** @brief 将字节数组以十六进制格式复制到剪贴板
     *  @param data 原始字节数据
     *  @param source 来源标识（可选） */
    void copyHex(const QByteArray &data, const QString &source = QString());
    /** @brief 将字节数组以Base64编码格式复制到剪贴板
     *  @param data 原始字节数据
     *  @param source 来源标识（可选） */
    void copyBase64(const QByteArray &data, const QString &source = QString());
    /** @brief 获取系统剪贴板当前文本内容
     *  @return 剪贴板中的文本，无内容时返回空串 */
    QString currentText() const;
    /** @brief 从系统剪贴板读取文本并返回
     *  @return 剪贴板文本内容 */
    QString pasteText() const;
    /** @brief 将文本转换为十六进制字符串表示
     *  @param text 输入文本
     *  @return 十六进制格式字符串（空格分隔） */
    static QString textToHex(const QString &text);
    /** @brief 将十六进制字符串解析为字节数组
     *  @param hexStr 十六进制字符串（支持空格分隔）
     *  @return 解析后的字节数组 */
    static QByteArray hexToBytes(const QString &hexStr);
    /** @brief 将字节数组编码为Base64字符串
     *  @param data 原始字节数据
     *  @return Base64编码字符串 */
    static QString bytesToBase64(const QByteArray &data);
    /** @brief 将Base64字符串解码为字节数组
     *  @param base64 Base64编码字符串
     *  @return 解码后的字节数组 */
    static QByteArray base64ToBytes(const QString &base64);
    /** @brief 将文本中的特殊字符转换为转义序列表示
     *  @param text 输入文本
     *  @return 包含转义序列的文本（如 \\n \\r \\t） */
    static QString textToEscape(const QString &text);
    /** @brief 获取完整的剪贴板历史记录
     *  @return 历史条目列表（按时间倒序） */
    QList<ClipboardEntry> history() const;
    /** @brief 获取最近N条剪贴板历史记录
     *  @param count 需要获取的条目数量
     *  @return 最近的条目列表 */
    QList<ClipboardEntry> recentHistory(int count) const;
    /** @brief 清空所有剪贴板历史记录 */
    void clearHistory();
    /** @brief 获取当前历史记录条数
     *  @return 历史记录数量 */
    int historySize() const;
    /** @brief 设置历史记录最大保留条数
     *  @param maxSize 最大条数，超出后自动淘汰最旧记录 */
    void setMaxHistorySize(int maxSize);
    /** @brief 获取历史记录最大保留条数
     *  @return 当前设置的最大条数 */
    int maxHistorySize() const;
    /** @brief 从历史记录中恢复指定索引的内容到剪贴板
     *  @param index 历史记录索引 */
    void restoreFromHistory(int index);
    /** @brief 获取累计复制操作总次数 */
    quint64 totalCopyOps() const;
    /** @brief 获取累计粘贴操作总次数 */
    quint64 totalPasteOps() const;
    /** @brief 获取累计格式转换操作总次数 */
    quint64 totalConversions() const;
    quint64 totalHexConversions() const { return m_totalHexConversions; }     ///< 累计Hex格式复制次数
    quint64 totalBase64Conversions() const { return m_totalBase64Conversions; } ///< 累计Base64格式复制次数
    quint64 totalRestores() const { return m_totalRestores; }                 ///< 累计从历史恢复次数
    quint64 totalHistoryClears() const { return m_totalHistoryClears; }       ///< 累计清除历史次数
    quint64 totalBytesCopied() const { return m_totalBytesCopied; }           ///< 累计复制的字节总数
    quint64 totalEscapeConversions() const { return m_totalEscapeConversions; } ///< 累计转义格式转换次数
    quint64 totalHistoryDuplicates() const { return m_totalHistoryDuplicates; } ///< 累计历史去重合并次数
    quint64 totalEmptyCopySkips() const { return m_totalEmptyCopySkips; } ///< 获取因空内容跳过复制的次数
    quint64 totalHexPastes() const { return m_totalHexPastes; } ///< 获取从十六进制字符串还原字节的次数
    quint64 totalBase64Pastes() const { return m_totalBase64Pastes; } ///< 获取从Base64字符串还原字节的次数
    void resetStatistics();
signals:
    void historyEntryAdded(const ClipboardEntry &entry);
    void historyCleared();
    void clipboardContentChanged();
private:
    explicit ClipboardManager(QObject *parent = nullptr);
    ~ClipboardManager() override;
    ClipboardManager(const ClipboardManager &) = delete;
    ClipboardManager &operator=(const ClipboardManager &) = delete;
    void addHistoryEntry(const ClipboardEntry &entry);
    QList<ClipboardEntry> m_history;
    int m_maxHistorySize = 100;
    mutable quint64 m_totalCopyOps = 0;
    mutable quint64 m_totalPasteOps = 0;
    mutable quint64 m_totalConversions = 0;
    quint64 m_totalHexConversions = 0;          ///< 累计Hex格式复制次数
    quint64 m_totalBase64Conversions = 0;       ///< 累计Base64格式复制次数
    quint64 m_totalRestores = 0;                ///< 累计从历史恢复次数
    quint64 m_totalHistoryClears = 0;           ///< 累计清除历史次数
    quint64 m_totalBytesCopied = 0;             ///< 累计复制的字节总数
    quint64 m_totalEscapeConversions = 0;       ///< 累计转义格式转换次数
    quint64 m_totalHistoryDuplicates = 0;       ///< 累计历史去重合并次数
    quint64 m_totalEmptyCopySkips = 0;          ///< 累计因空内容跳过复制的次数
    quint64 m_totalHexPastes = 0;               ///< 累计从十六进制字符串还原字节的次数
    quint64 m_totalBase64Pastes = 0;            ///< 累计从Base64字符串还原字节的次数
};
#endif // CLIPBOARDMANAGER_H
