#ifndef TOOLBARCONTROLLER_H
#define TOOLBARCONTROLLER_H

#include <QObject>
#include <QStringList>

class QMainWindow;
class QToolBar;
class QComboBox;
class QAction;
class RecordingController;

/**
 * @brief 工具栏控制器 - 管理主工具栏所有控件的创建和事件转发
 *
 * 职责:
 *   1. 创建工具栏的所有控件（显示模式/时间戳/方向前缀/清屏/导出/背景/主题/语言/终端布局）
 *   2. 为所有控件设置 objectName（供 QSS 选择器使用）
 *   3. 将工具栏控件的交互事件通过信号转发给 MainWindow
 *   4. 管理主题和语言下拉框的数据模型
 *
 * 设计模式:
 *   - 外观模式(Facade): 将工具栏控件的创建和信号转发封装在一个类中
 *   - 观察者模式: 通过 Qt 信号/槽转发工具栏事件
 *
 * 协作关系:
 *   - RecordingController: 录制/回放按钮由其管理
 *   - SettingsController: 接收主题/语言变更事件
 *   - ThemeManager: 获取可用主题列表
 *   - MainWindow: 接收所有工具栏信号并分发到对应处理方法
 */
class ToolbarController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造工具栏控制器
     * @param recordingController 录制控制器，用于在工具栏中添加录制/回放按钮
     * @param parent 父对象
     */
    explicit ToolbarController(RecordingController* recordingController, QObject* parent = nullptr);

    /**
     * @brief 创建并返回工具栏，添加到主窗口
     * 控件布局: 显示模式 | 时间戳 [TX/RX] | 清屏 | 导出 背景 | 录制/回放 | 终端布局 | 主题 语言
     * @param parent 主窗口实例
     * @return 创建的工具栏指针
     */
    QToolBar* createToolbar(QMainWindow* parent);

    /**
     * @brief 创建显示模式相关控件组
     * 包含: 显示模式下拉框(文本/HEX/混合/十进制)、终端布局下拉框、时间戳开关、方向前缀开关、清屏按钮
     * @param toolbar 目标工具栏
     */
    void createDisplayModeGroup(QToolBar* toolbar);

    /**
     * @brief 创建连接相关控制组
     * 包含: 导出按钮、背景设置按钮、录制/回放(委托RecordingController)、主题/语言下拉框
     * @param toolbar 目标工具栏
     */
    void createConnectionGroup(QToolBar* toolbar);

    /**
     * @brief 获取工具栏指针
     * 用于定位弹出面板（如背景设置面板需要在工具栏下方弹出）
     * @return 工具栏指针
     */
    QToolBar* toolbar() const;

    /**
     * @brief 设置可用主题列表
     * 用 ThemeManager 的主题名列表初始化主题下拉框
     * @param themes 主题名称列表（如 ["dark_terminal", "modern_dark", "light"]）
     */
    void setAvailableThemes(const QStringList& themes);

    /**
     * @brief 设置当前选中的主题
     * 通过主题名称匹配下拉框中的项并设置选中
     * @param themeName 主题名称
     */
    void setCurrentTheme(const QString& themeName);

    /**
     * @brief 根据索引获取主题名称
     * @param index 下拉框索引
     * @return 主题名称，索引无效时返回空字符串
     */
    QString themeNameAt(int index) const;

    /**
     * @brief 设置当前选中的语言
     * @param langCode 语言代码（如 "zh_CN"、"en"）
     */
    void setCurrentLanguage(const QString& langCode);

    /**
     * @brief 根据索引获取语言代码
     * @param index 下拉框索引
     * @return 语言代码，索引无效时返回空字符串
     */
    QString languageCodeAt(int index) const;

signals:
    /** @brief 显示模式下拉框索引变更 */
    void displayModeChanged(int index);

    /** @brief 时间戳开关切换 */
    void timestampToggled(bool checked);

    /** @brief 收发方向前缀开关切换 */
    void dirPrefixToggled(bool checked);

    /** @brief 清屏按钮点击 */
    void clearRequested();

    /** @brief 导出按钮点击 */
    void exportRequested();

    /** @brief 背景设置按钮点击 */
    void bgSettingsRequested();

    /** @brief 主题下拉框索引变更 */
    void themeChanged(int index);

    /** @brief 语言下拉框索引变更 */
    void languageChanged(int index);

    /** @brief 终端布局模式下拉框索引变更: 0=混合, 1=左右分栏, 2=上下分栏 */
    void terminalLayoutChanged(int index);

private:
    /** @brief 录制控制器，用于在工具栏中添加录制/回放按钮 */
    RecordingController* m_recordingController;

    // ---- 工具栏控件 ----

    /** @brief 主工具栏实例 */
    QToolBar* m_toolbar;

    /** @brief 显示模式下拉框: 文本/HEX/混合/十进制 */
    QComboBox* m_displayModeCombo;

    /** @brief 终端布局模式下拉框: 混合/左右分栏/上下分栏 */
    QComboBox* m_layoutCombo;

    /** @brief 主题选择下拉框 */
    QComboBox* m_themeCombo;

    /** @brief 语言选择下拉框 */
    QComboBox* m_langCombo;

    /** @brief 时间戳显示开关（可切换 Action） */
    QAction* m_timestampAction;

    /** @brief 收发方向前缀开关（可切换 Action） */
    QAction* m_dirPrefixAction;

    /** @brief 清屏按钮 */
    QAction* m_clearAction;

    /** @brief 数据导出按钮 */
    QAction* m_exportAction;

    /** @brief 背景设置按钮 */
    QAction* m_bgAction;
};

#endif // TOOLBARCONTROLLER_H
