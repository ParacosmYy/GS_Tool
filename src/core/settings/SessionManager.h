/**
 * @file SessionManager.h
 * @brief 会话管理器 - 保存和恢复用户完整工作区状态
 *
 * 职责:
 *   1. 保存当前工作区: 串口配置 + 面板状态 + 窗口几何
 *   2. 恢复上次工作区: 从持久化存储中读取并还原所有状态
 *   3. 作为工作区快照的统一入口，内部委托 SettingsManager 做实际存储
 *
 * 设计模式:
 *   - 门面模式 (Facade): 将分散在多个模块的状态收集和恢复统一到 saveSession/loadSession
 *   - 委托模式: 持久化操作委托给 SettingsManager
 *
 * 协作关系:
 *   - SettingsManager: 底层配置持久化单例
 *   - SerialConfigPanel: 串口配置参数的读取/恢复
 *   - SettingsController: 窗口几何/主题/面板索引的保存/恢复
 *
 * 所属层级: 业务层（协调多个模块完成工作区快照功能）
 */

#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>

class QMainWindow;
class SerialConfigPanel;
class SettingsController;

/**
 * @brief 会话管理器 - 保存和恢复用户完整工作区状态
 *
 * 一次会话（Session）包含以下状态快照:
 *   - 串口配置: 端口名/波特率/数据位/校验/停止位/流控/DTR/RTS
 *   - 窗口几何: 位置和大小
 *   - 主题偏好: 当前使用的主题名称
 *   - 上次面板: 上次查看的功能面板索引
 *
 * 使用方式:
 * @code
 *   auto* sessionMgr = new SessionManager(mainWindow, this);
 *   sessionMgr->setSerialConfigPanel(serialPanel);
 *   sessionMgr->setSettingsController(settingsCtrl);
 *   // 保存完整工作区
 *   sessionMgr->saveSession();
 *   // 恢复完整工作区
 *   sessionMgr->loadSession();
 * @endcode
 */
class SessionManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造会话管理器
     *
     * @param mainWindow 主窗口实例，用于保存/恢复窗口几何
     * @param parent 父对象
     */
    explicit SessionManager(QMainWindow* mainWindow, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~SessionManager() override = default;

    // 禁止拷贝和赋值
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    /**
     * @brief 注入串口配置面板引用
     *
     * 用于从面板收集当前串口配置参数，或将保存的配置恢复到面板。
     * 必须在调用 saveSession/loadSession 之前设置。
     *
     * @param panel 串口配置面板指针
     */
    void setSerialConfigPanel(SerialConfigPanel* panel);

    /**
     * @brief 注入设置控制器引用
     *
     * 用于保存/恢复窗口几何、主题、面板索引等设置项。
     * 必须在调用 saveSession/loadSession 之前设置。
     *
     * @param controller 设置控制器指针
     */
    void setSettingsController(SettingsController* controller);

    /**
     * @brief 保存当前完整工作区到磁盘
     *
     * 保存内容包括:
     *   1. 窗口几何（位置和大小）
     *   2. 串口配置（端口名/波特率/数据位/校验/停止位/流控/DTR/RTS）
     *   3. 当前主题名称
     *   4. 上次活跃面板索引
     *
     * 所有数据通过 SettingsManager 写入配置文件，最后调用 sync() 确保落盘。
     */
    void saveSession();

    /**
     * @brief 从磁盘恢复上次保存的工作区
     *
     * 恢复顺序:
     *   1. 窗口几何 → 恢复窗口位置和大小
     *   2. 主题 → 应用保存的主题
     *   3. 串口配置 → 恢复到配置面板
     *   4. 上次面板索引 → 返回给调用方用于面板切换
     *
     * @return 上次活跃的面板索引，无保存数据时返回 -1
     */
    int loadSession();

    // ---- 统计计数器 ----

    /** @brief 获取会话保存总次数 @return 保存操作总次数 */
    quint64 totalSaves() const;

    /** @brief 获取会话加载总次数 @return 加载操作总次数 */
    quint64 totalLoads() const;

    /** @brief 获取会话操作错误总次数 @return 错误次数 */
    quint64 errorCount() const;

    /** @brief 获取累计会话创建次数 @return 会话创建次数 */
    quint64 totalSessionsCreated() const;

    /** @brief 获取累计会话删除次数 @return 会话删除次数 */
    quint64 totalSessionDeletes() const;

    /** @brief 重置所有会话管理统计计数器为零 */
    void resetSessionStatistics();

private:
    /** @brief 主窗口实例，用于保存/恢复窗口几何 */
    QMainWindow* m_mainWindow;

    /** @brief 串口配置面板，用于收集/恢复串口参数 */
    SerialConfigPanel* m_serialConfig;

    /** @brief 设置控制器，用于窗口几何/主题/面板索引的保存/恢复 */
    SettingsController* m_settingsController;

    // ---- 统计计数器 ----
    quint64 m_totalSaves = 0;          ///< 会话保存总次数
    quint64 m_totalLoads = 0;          ///< 会话加载总次数
    quint64 m_errorCount = 0;          ///< 会话操作错误总次数
    quint64 m_totalSessionsCreated = 0; ///< 累计会话创建次数
    quint64 m_totalSessionDeletes = 0;  ///< 累计会话删除次数
};

#endif // SESSIONMANAGER_H
