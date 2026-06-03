/**
 * @file ResponsiveLayout.h
 * @brief 响应式布局管理器 -- 监听窗口尺寸变化，自动切换断点并通知UI层适配
 *
 * 职责:
 *   1. 通过事件过滤器监听 QMainWindow 的 Resize 事件
 *   2. 根据窗口宽度判定当前断点（Compact / Medium / Desktop）
 *   3. 断点变化时发射 breakpointChanged 信号，通知各组件自适应布局
 *   4. 在窗口上设置 "breakpoint" 属性，供 QSS 选择器使用
 *
 * 断点定义:
 *   - Compact (< 900px):  仅图标侧边栏，隐藏文字标签
 *   - Medium  (900~1200px): 可折叠侧边栏，精简布局
 *   - Desktop (>= 1200px): 完整侧边栏，全功能布局
 *
 * 协作关系:
 *   - MainWindow: 启动时调用 watchWindow() 注册监听
 *   - NavigationController: 监听 breakpointChanged 调整侧边栏宽度
 *   - QSS样式: 通过 [breakpoint="compact"] 等属性选择器适配样式
 */

#ifndef RESPONSIVELAYOUT_H
#define RESPONSIVELAYOUT_H

#include <QObject>
#include <QString>

class QMainWindow;
class QEvent;

/**
 * @brief 响应式布局管理器
 *
 * 安装在 QMainWindow 上的事件过滤器，监听窗口尺寸变化并划分断点。
 * 使用方式: MainWindow 构造时调用 watchWindow(this) 即可自动运行。
 */
class ResponsiveLayout : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 布局断点枚举
     *
     * 对应不同窗口宽度范围下的布局策略。
     * 断点名称会设为窗口属性，供 QSS 选择器使用。
     */
    enum class Breakpoint {
        Compact,    ///< < 900px -- 仅图标侧边栏
        Medium,     ///< 900~1200px -- 可折叠侧边栏
        Desktop     ///< >= 1200px -- 完整侧边栏
    };
    Q_ENUM(Breakpoint)

    /**
     * @brief 构造函数
     * @param parent 父对象（通常为 nullptr，由 MainWindow 手动持有）
     */
    explicit ResponsiveLayout(QObject* parent = nullptr);

    /**
     * @brief 监听指定主窗口的尺寸变化
     *
     * 安装事件过滤器到目标窗口，并立即根据当前窗口宽度初始化断点。
     * 重复调用会先移除对旧窗口的监听。
     *
     * @param window 要监听的主窗口指针
     */
    void watchWindow(QMainWindow* window);

    /**
     * @brief 获取当前断点
     * @return 当前激活的断点枚举值
     */
    Breakpoint currentBreakpoint() const;

    /**
     * @brief 获取断点名称字符串，用于 QSS 属性选择器
     *
     * 返回值示例: "compact" / "medium" / "desktop"
     *
     * @return 小写的断点名称
     */
    QString breakpointName() const;

signals:
    /**
     * @brief 断点变化信号
     *
     * 当窗口宽度跨越断点阈值时发射。UI 组件应监听此信号调整布局。
     *
     * @param newBreakpoint 新断点
     * @param oldBreakpoint 旧断点
     */
    void breakpointChanged(Breakpoint newBreakpoint, Breakpoint oldBreakpoint);

protected:
    /**
     * @brief 事件过滤器 -- 拦截被监听窗口的 Resize 事件
     * @param watched 事件接收对象
     * @param event 事件对象
     * @return false（不拦截事件，仅监听）
     */
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    /**
     * @brief 根据窗口宽度更新断点
     *
     * 比较新断点与当前断点，若不同则发射 breakpointChanged 信号
     * 并更新窗口的 "breakpoint" 属性。
     *
     * @param width 窗口新宽度（像素）
     */
    void updateBreakpoint(int width);

    Breakpoint m_currentBreakpoint = Breakpoint::Desktop;  ///< 当前激活断点
    QMainWindow* m_window = nullptr;                        ///< 被监听的主窗口
};

#endif // RESPONSIVELAYOUT_H
