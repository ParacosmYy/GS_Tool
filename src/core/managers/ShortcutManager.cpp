/**
 * @file ShortcutManager.cpp
 * @brief 统一键盘快捷键管理器实现
 *
 * 单例模式，集中管理所有快捷键的注册、冲突检测和上下文控制。
 * 调用方通过 registerShortcut() 注册快捷键并提供回调。
 */

#include "core/managers/ShortcutManager.h"

#include <QWidget>
#include <QHash>

// ─── 单例 ────────────────────────────────────────────────

/** @brief 获取ShortcutManager单例实例 @return 单例引用 */
ShortcutManager& ShortcutManager::instance()
{
    static ShortcutManager s_instance;
    return s_instance;
}

/** @brief 构造快捷键管理器 @param parent 父对象指针 */
ShortcutManager::ShortcutManager(QObject* parent)
    : QObject(parent)
{
}

/** @brief 析构函数，QShortcut对象由parent widget管理无需手动释放 */
ShortcutManager::~ShortcutManager()
{
}

// ─── 注册 ────────────────────────────────────────────────

/**
 * @brief 注册一个新的快捷键绑定
 * @param id 快捷键唯一标识符，已存在时注册失败
 * @param key 快捷键序列，已被占用时注册失败
 * @param parent 快捷键所属的父控件
 * @param callback 快捷键激活时的回调函数
 * @param description 快捷键描述文本，用于工具提示显示
 * @param context 快捷键上下文模式
 * @return 注册成功返回true，ID或按键冲突时返回false
 */
bool ShortcutManager::registerShortcut(const QString& id,
                                        const QKeySequence& key,
                                        QWidget* parent,
                                        std::function<void()> callback,
                                        const QString& description,
                                        ShortcutContext context)
{
    ++m_totalRegistrations;  ///< 统计: 快捷键注册尝试次数递增

    // ID 冲突检查
    if (m_shortcuts.contains(id)) {
        return false;
    }

    // 按键冲突检查
    if (isKeyOccupied(key)) {
        return false;
    }

    // 创建 QShortcut 对象
    auto* shortcut = new QShortcut(key, parent);
    shortcut->setContext(Qt::ApplicationShortcut);

    // 连接回调
    QObject::connect(shortcut, &QShortcut::activated, this, [this, callback]() {
        ++m_totalTriggers;  ///< 统计: 快捷键触发次数递增
        if (callback) {
            callback();
        }
    });

    // 存储信息
    ShortcutInfo info;
    info.id = id;
    info.keySequence = key;
    info.description = description;
    info.context = context;
    info.shortcut = shortcut;

    m_shortcuts.insert(id, info);
    return true;
}

// ─── 注销 ────────────────────────────────────────────────

/**
 * @brief 注销指定ID的快捷键并删除QShortcut对象
 * @param id 要注销的快捷键唯一标识符
 */
void ShortcutManager::unregisterShortcut(const QString& id)
{
    auto it = m_shortcuts.find(id);
    if (it == m_shortcuts.end()) {
        return;
    }

    // 删除 QShortcut 对象（Qt 父子树会自动管理，但显式删除更安全）
    if (it->shortcut) {
        delete it->shortcut;
    }

    m_shortcuts.erase(it);
}

// ─── 修改绑定 ────────────────────────────────────────────

/**
 * @brief 修改指定快捷键的按键绑定
 * @param id 要修改的快捷键唯一标识符
 * @param newKey 新的按键序列
 * @return 修改成功返回true，ID不存在或新按键被占用时返回false
 */
bool ShortcutManager::rebind(const QString& id, const QKeySequence& newKey)
{
    auto it = m_shortcuts.find(id);
    if (it == m_shortcuts.end()) {
        return false;
    }

    // 检查新按键是否被其他快捷键占用（排除自身）
    if (isKeyOccupied(newKey, id)) {
        return false;
    }

    // 更新按键序列
    it->keySequence = newKey;
    if (it->shortcut) {
        it->shortcut->setKey(newKey);
    }

    emit shortcutChanged(id, newKey);
    return true;
}

// ─── 查询 ────────────────────────────────────────────────

/** @brief 获取所有已注册快捷键的信息列表 @return 快捷键信息列表 */
QList<ShortcutInfo> ShortcutManager::allShortcuts() const
{
    return m_shortcuts.values();
}

/**
 * @brief 获取指定快捷键的工具提示文本，格式为"描述 (按键)"
 * @param id 快捷键唯一标识符
 * @return 格式化的工具提示字符串，ID不存在时返回空字符串
 */
QString ShortcutManager::shortcutTooltip(const QString& id) const
{
    auto it = m_shortcuts.constFind(id);
    if (it == m_shortcuts.constEnd()) {
        return {};
    }

    // 格式: "描述 (Ctrl+F)"
    return tr("%1 (%2)")
        .arg(it->description,
             it->keySequence.toString(QKeySequence::NativeText));
}

/**
 * @brief 检查指定按键序列是否已被其他快捷键占用
 * @param key 待检查的按键序列
 * @param excludeId 需要排除检查的快捷键ID，默认为空
 * @return 已被占用返回true，否则返回false
 */
bool ShortcutManager::isKeyOccupied(const QKeySequence& key,
                                     const QString& excludeId) const
{
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
        if (it.key() == excludeId) {
            continue;
        }
        if (it->keySequence == key) {
            return true;
        }
    }
    return false;
}

// ---- 统计计数器实现 ----

/** @brief 获取快捷键注册总次数 @return 注册操作总次数 */
quint64 ShortcutManager::totalRegistrations() const
{
    return m_totalRegistrations;
}

/** @brief 获取快捷键触发总次数 @return 触发总次数 */
quint64 ShortcutManager::totalTriggers() const
{
    return m_totalTriggers;
}

/** @brief 重置所有快捷键管理统计计数器为零 */
void ShortcutManager::resetShortcutStatistics()
{
    m_totalRegistrations = 0;
    m_totalTriggers = 0;
}
