/**
 * @file ShortcutManager2.h
 * @brief 快捷键管理器 - 统一注册、查询和重绑定全局快捷键
 *
 * 职责:
 *   1. 集中管理所有快捷键的注册信息（ID、标签、默认键、当前键、分类）
 *   2. 支持运行时重绑定快捷键
 *   3. 检测快捷键冲突
 *   4. 按分类查询和批量重置
 */

#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QKeySequence>

/**
 * @brief 快捷键管理器
 *
 * 通过 registerShortcut() 注册快捷键条目，rebind() 修改绑定，
 * hasConflict() 检测冲突。所有快捷键按唯一ID索引，支持按分类查询。
 */
class ShortcutManager : public QObject {
    Q_OBJECT
public:
    /** @brief 快捷键条目结构体 */
    struct ShortcutEntry {
        QString id;                 ///< 快捷键唯一标识符
        QString label;              ///< 用户可读的标签文本
        QKeySequence defaultKey;    ///< 默认快捷键
        QKeySequence currentKey;    ///< 当前绑定的快捷键
        QString category;           ///< 所属分类（如 "General"、"编辑" 等）
    };

    /**
     * @brief 构造快捷键管理器
     * @param parent 父对象
     */
    explicit ShortcutManager(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~ShortcutManager() override;

    /**
     * @brief 注册一条快捷键
     * @param id 唯一标识符
     * @param label 用户可读标签
     * @param defaultKey 默认快捷键
     * @param category 所属分类，默认 "General"
     */
    void registerShortcut(const QString &id, const QString &label,
                          const QKeySequence &defaultKey, const QString &category = "General");

    /**
     * @brief 移除已注册的快捷键
     * @param id 快捷键标识符
     */
    void unregisterShortcut(const QString &id);

    /**
     * @brief 重新绑定快捷键到新的按键序列
     * @param id 快捷键标识符
     * @param newKey 新的按键序列
     */
    void rebind(const QString &id, const QKeySequence &newKey);

    /**
     * @brief 将指定快捷键恢复为默认绑定
     * @param id 快捷键标识符
     */
    void resetToDefault(const QString &id);

    /** @brief 将所有快捷键恢复为默认绑定 */
    void resetAll();

    /**
     * @brief 获取指定快捷键的当前绑定
     * @param id 快捷键标识符
     * @return 当前按键序列
     */
    QKeySequence shortcut(const QString &id) const;

    /**
     * @brief 获取指定快捷键的用户可读标签
     * @param id 快捷键标识符
     * @return 标签文本
     */
    QString shortcutLabel(const QString &id) const;

    /**
     * @brief 获取所有已注册快捷键
     * @return 快捷键条目列表
     */
    QList<ShortcutEntry> allShortcuts() const;

    /**
     * @brief 按分类获取快捷键
     * @param cat 分类名称
     * @return 该分类下的快捷键条目列表
     */
    QList<ShortcutEntry> shortcutsByCategory(const QString &cat) const;

    /**
     * @brief 获取所有分类名称
     * @return 分类名称列表（去重）
     */
    QStringList categories() const;

    /**
     * @brief 检测按键序列是否与已有快捷键冲突
     * @param key 要检测的按键序列
     * @param conflictId 输出参数，冲突的快捷键ID（可选）
     * @return true表示存在冲突
     */
    bool hasConflict(const QKeySequence &key, QString *conflictId = nullptr) const;

signals:
    /** @brief 新快捷键注册完成 @param id 快捷键标识符 */
    void shortcutRegistered(const QString &id);

    /** @brief 快捷键重新绑定 @param id 快捷键标识符 @param newKey 新的按键序列 */
    void shortcutRebound(const QString &id, const QKeySequence &newKey);

    /** @brief 检测到快捷键冲突 @param id1 第一个冲突的快捷键ID @param id2 第二个冲突的快捷键ID */
    void conflictDetected(const QString &id1, const QString &id2);

private:
    QMap<QString, ShortcutEntry> m_shortcuts; ///< 快捷键注册表（按ID索引）
};
