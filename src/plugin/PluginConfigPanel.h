/**
 * @file PluginConfigPanel.h
 * @brief 插件配置面板 — 插件管理 UI 控件
 *
 * 显示已加载和可用的插件列表，支持加载/卸载/扫描操作。
 * 用户交互通过信号通知外部 PluginManager 执行实际操作。
 *
 * 协作关系:
 *   - PluginManager: 响应面板信号执行插件加载/卸载
 */
#ifndef PLUGINCONFIGPANEL_H
#define PLUGINCONFIGPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QString>

class PluginManager;

/**
 * @brief 插件配置面板统计计数器
 *
 * 追踪面板展示、配置操作（加载/卸载/扫描）和选择变化指标。
 */
struct PluginConfigPanelStats {
    quint64 totalDisplays = 0;          ///< 累计面板显示/刷新次数(refreshList调用)
    quint64 totalPluginLoads = 0;       ///< 累计加载按钮点击次数
    quint64 totalPluginUnloads = 0;     ///< 累计卸载按钮点击次数
    quint64 totalScans = 0;             ///< 累计扫描次数
    quint64 totalConfigChanges = 0;     ///< 累计配置变更次数(管理器绑定/切换)
    quint64 pluginSelectionChanges = 0; ///< 累计插件列表选择变化次数
};

/**
 * @brief 插件配置面板
 *
 * 左侧为插件列表，右侧为加载/卸载按钮。
 * 列表项显示插件名称、版本和加载状态。
 */
class PluginConfigPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit PluginConfigPanel(QWidget* parent = nullptr);

    /**
     * @brief 绑定插件管理器
     * @param manager PluginManager 实例（非空）
     */
    void setPluginManager(PluginManager* manager);

    // ---- Stats struct 接口 ----

    /** @brief 获取统计计数器只读引用 @return 当前统计快照 */
    const PluginConfigPanelStats& stats() const { return m_stats; }

    /** @brief 重置所有面板统计计数器为初始值 */
    void resetStats() { m_stats = PluginConfigPanelStats{}; }

    // ---- 兼容性 getter（委托给 m_stats） ----

    /** @brief 获取累计加载按钮点击次数 */
    quint64 totalPluginLoads() const { return m_stats.totalPluginLoads; }
    /** @brief 获取累计卸载按钮点击次数 */
    quint64 totalPluginUnloads() const { return m_stats.totalPluginUnloads; }
    /** @brief 获取累计扫描次数 */
    quint64 totalScans() const { return m_stats.totalScans; }
    /** @brief 重置面板统计计数器归零（兼容旧接口） */
    void resetPluginPanelStatistics() { resetStats(); }

signals:
    /**
     * @brief 用户请求加载指定插件
     * @param filePath 插件文件路径
     */
    void loadPluginRequested(const QString& filePath);

    /**
     * @brief 用户请求卸载指定插件
     * @param name 插件名称
     */
    void unloadPluginRequested(const QString& name);

private:
    /** @brief 初始化 UI 布局和控件 */
    void setupUI();

    /** @brief 从 PluginManager 刷新插件列表显示 */
    void refreshList();

    /** @brief 更新右侧详情面板 */
    void updateDetailPanel(const QString& pluginName);

    QLabel* m_titleLabel;               ///< 标题标签
    QListWidget* m_pluginList;          ///< 插件列表控件
    QPushButton* m_loadBtn;             ///< 加载插件按钮
    QPushButton* m_unloadBtn;           ///< 卸载插件按钮
    QPushButton* m_scanBtn;             ///< 扫描插件按钮
    QLabel* m_detailNameLabel;          ///< 详情-插件名称
    QLabel* m_detailVersionLabel;       ///< 详情-版本号
    QTextEdit* m_detailDescEdit;        ///< 详情-描述信息
    QLabel* m_detailStatusLabel;        ///< 详情-加载状态
    PluginManager* m_manager = nullptr; ///< 插件管理器（不拥有）

    PluginConfigPanelStats m_stats;     ///< 面板统计计数器
};

#endif // PLUGINCONFIGPANEL_H
