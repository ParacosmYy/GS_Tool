/**
 * @file ResponsiveLayout.h
 * @brief 响应式布局管理器 -- 四断点体系，监听窗口尺寸自动切换布局
 *
 * 四断点: Mobile(<768) / Tablet(768~1023) / Desktop(1024~1439) / Wide(>=1440)
 * 导航自动折叠: 窗口宽度 < 900px 时自动折叠导航树（独立于四断点）
 * 协作: MainWindow(watchWindow) / NavigationController(navTreeAutoCollapse) / QSS([breakpoint])
 */

#ifndef RESPONSIVELAYOUT_H
#define RESPONSIVELAYOUT_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QPropertyAnimation>
#include <QEasingCurve>

class QMainWindow;
class QEvent;
class QWidget;

/**
 * @brief 响应式布局管理器
 *
 * 安装在 QMainWindow 上的事件过滤器，监听窗口尺寸变化并划分断点。
 * 使用方式: MainWindow 构造时调用 watchWindow(this) 即可自动运行。
 */
class ResponsiveLayout : public QObject {
    Q_OBJECT

public:
    /** @brief 布局断点枚举 -- 对应不同窗口宽度范围的布局策略 */
    enum class Breakpoint {
        Mobile,    ///< < 768px -- 仅图标栏，隐藏导航树
        Tablet,    ///< 768~1023px -- 可折叠侧边栏
        Desktop,   ///< 1024~1439px -- 标准桌面布局
        Wide       ///< >= 1440px -- 宽屏全功能布局
    };
    Q_ENUM(Breakpoint)

    /** @brief Widget 在不同断点下的可见性策略 */
    enum class Visibility {
        Visible,    ///< 显示
        Hidden,     ///< 隐藏（setVisible(false)）
        Collapsed   ///< 折叠（宽度压缩为0）
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit ResponsiveLayout(QObject* parent = nullptr);

    /** @brief 监听指定主窗口的尺寸变化，安装事件过滤器并初始化断点 @param window 主窗口 */
    void watchWindow(QMainWindow* window);

    /** @brief 获取当前断点 @return 当前激活的断点枚举值 */
    Breakpoint currentBreakpoint() const;

    /** @brief 获取断点名称字符串，供 QSS 属性选择器使用 @return "mobile"/"tablet"/"desktop"/"wide" */
    QString breakpointName() const;

    /** @brief 导航树是否应自动折叠（< 900px） @return true 应折叠 */
    bool isNavTreeCollapsed() const;

    /** @brief 注册 widget 的断点可见性策略，断点切换时自动应用 @param widget 目标 @param policy 断点→可见性 */
    void registerWidgetVisibility(QWidget* widget,
                                  const QMap<Breakpoint, Visibility>& policy);

    /** @brief 移除 widget 的可见性策略注册 @param widget 目标 */
    void unregisterWidgetVisibility(QWidget* widget);

    /** @brief 保存布局配置到 QSettings（断点、导航折叠状态） */
    void saveLayoutConfig() const;

    /** @brief 从 QSettings 加载布局配置 */
    void loadLayoutConfig();

    // ---- 统计计数器 ----
    /** @brief 布局变更总次数(含resize) @return 总次数 */
    quint64 totalLayoutChanges() const;
    /** @brief 断点切换总次数 @return 总次数 */
    quint64 breakpointChangeCount() const;
    /** @brief 导航树折叠/展开切换总次数 @return 总次数 */
    quint64 navCollapseToggleCount() const;
    /** @brief 获取Widget可见性策略注册总次数 @return 注册计数 */
    quint64 totalWidgetRegistrations() const { return m_totalWidgetRegistrations; }
    /** @brief 重置所有统计计数器 */
    void resetStats();

signals:
    /** @brief 断点变化信号 @param newBp 新断点 @param oldBp 旧断点 */
    void breakpointChanged(Breakpoint newBp, Breakpoint oldBp);

    /** @brief 导航树自动折叠状态变化信号（跨越 900px） @param collapsed true 应折叠 */
    void navTreeAutoCollapse(bool collapsed);

    /** @brief 断点过渡动画开始信号 @param newBp 目标断点 */
    void breakpointTransitionStarted(Breakpoint newBp);

protected:
    /** @brief 事件过滤器 -- 拦截 Resize 事件 @return false 不拦截 */
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    /** @brief 根据窗口宽度更新断点 @param width 像素 */
    void updateBreakpoint(int width);
    /** @brief 应用当前断点的可见性策略 @param bp 断点 */
    void applyVisibilityPolicy(Breakpoint bp);
    /** @brief 根据宽度计算断点（纯计算） @return 断点 */
    static Breakpoint calculateBreakpoint(int width);
    /** @brief 启动断点过渡动画（透明度脉冲） @param newBp 目标断点 */
    void startTransitionAnimation(Breakpoint newBp);
    /** @brief 根据宽度判断导航树是否应折叠 @return true < 900px */
    static bool shouldCollapseNav(int width);

    Breakpoint m_currentBreakpoint = Breakpoint::Desktop;  ///< 当前断点
    QMainWindow* m_window = nullptr;                        ///< 被监听窗口
    bool m_navCollapsed = false;                            ///< 导航折叠状态

    /** @brief widget 可见性策略注册表 */
    QMap<QWidget*, QMap<Breakpoint, Visibility>> m_visibilityPolicies;

    /** @brief 断点过渡动画实例 */
    QPropertyAnimation* m_transitionAnim = nullptr;

    // ---- 统计 ----
    quint64 m_totalLayoutChanges = 0;      ///< resize触发布局变更次数
    quint64 m_breakpointChangeCount = 0;   ///< 断点实际切换次数
    quint64 m_navCollapseToggleCount = 0;  ///< 导航折叠切换次数
    quint64 m_totalWidgetRegistrations = 0; ///< Widget可见性策略注册次数
};

#endif // RESPONSIVELAYOUT_H
