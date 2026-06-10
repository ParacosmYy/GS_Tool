/**
 * @file ScriptEditorWidget.h
 * @brief 脚本编辑器面板 — 代码编辑、脚本管理、参数配置、输出控制台
 *
 * 布局: 左侧脚本列表+增删按钮 | 右侧代码编辑器+参数表+输出控制台+运行/停止按钮。
 * 统计见 ScriptEditorWidgetStats.cpp。
 * 协作: ScriptEngine(执行引擎) / PanelManager(面板集成)
 */
#ifndef SCRIPTEDITORWIDGET_H
#define SCRIPTEDITORWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QTableWidget>
#include <QSplitter>

#include "utils/scripting/ScriptEngine.h"

/**
 * @brief 脚本编辑器面板 — 代码编辑 + 脚本管理 + 参数表 + 输出控制台
 *
 * 左右分栏布局，左侧为脚本列表(增删改)，右侧为代码编辑器、参数表和输出控制台。
 * 所有 QWidget 均设置 objectName，所有用户可见字符串使用 tr()。
 */
class ScriptEditorWidget : public QWidget {
    Q_OBJECT

public:
    explicit ScriptEditorWidget(QWidget* parent = nullptr);

    /** @brief 设置脚本引擎(必须在使用前调用) */
    void setEngine(ScriptEngine* engine);

    /** @brief 获取当前编辑器中的脚本代码 */
    QString currentCode() const;

    /** @brief 获取当前选中的脚本索引(-1 表示未选中) */
    int currentScriptIndex() const;

    // ---- 统计 ----
    QVariantMap stats() const;              ///< 获取面板统计信息
    void resetPanelStatistics();            ///< 重置面板统计

signals:
    /** @brief 请求发送数据(脚本 serialWrite 调用) */
    void sendDataRequested(const QByteArray& data);

private slots:
    void onRunClicked();                    ///< 运行当前脚本
    void onStopClicked();                   ///< 停止执行(预留)
    void onAddScript();                     ///< 添加新脚本
    void onRemoveScript();                  ///< 删除选中脚本
    void onScriptSelected(int row);         ///< 脚本列表选中切换
    void onCodeChanged();                   ///< 代码编辑器内容变更
    void onScriptExecuted(const ScriptResult& result);  ///< 脚本执行结果回调
    void onOutputReady(const QString& text);            ///< 脚本输出回调
    void onErrorOccurred(const QString& message);       ///< 脚本错误回调

private:
    void setupUi();                         ///< 构建 UI 布局
    void connectSignals();                  ///< 连接信号槽
    void updateScriptList();                ///< 刷新脚本列表显示
    void saveCurrentScript();               ///< 保存当前编辑器内容到脚本列表
    void loadScriptToEditor(int index);     ///< 加载指定脚本到编辑器
    void appendOutput(const QString& text, const QString& color = {}); ///< 追加输出

    ScriptEngine* m_engine = nullptr;       ///< 脚本执行引擎(外部拥有)

    // ---- UI 组件 ----
    QSplitter* m_mainSplitter;              ///< 左右分栏
    QListWidget* m_scriptList;              ///< 脚本列表
    QPushButton* m_addBtn;                  ///< 添加脚本按钮
    QPushButton* m_removeBtn;               ///< 删除脚本按钮
    QPlainTextEdit* m_codeEditor;           ///< 代码编辑器
    QComboBox* m_languageCombo;             ///< 语言选择
    QTableWidget* m_paramTable;             ///< 参数表
    QPushButton* m_runBtn;                  ///< 运行按钮
    QPushButton* m_stopBtn;                 ///< 停止按钮
    QPlainTextEdit* m_outputConsole;        ///< 输出控制台
    QLabel* m_statusLabel;                  ///< 状态标签

    // ---- 状态 ----
    int m_currentIndex = -1;                ///< 当前编辑脚本索引
    bool m_updatingFromCode = false;        ///< 防止循环更新标志

    // ---- 面板统计 ----
    quint64 m_totalRuns = 0;                ///< 累计运行次数
    quint64 m_totalSuccessRuns = 0;         ///< 累计成功次数
    quint64 m_totalFailedRuns = 0;          ///< 累计失败次数
    quint64 m_totalScriptsAdded = 0;        ///< 累计添加脚本数
    quint64 m_totalScriptsRemoved = 0;      ///< 累计删除脚本数
};

#endif // SCRIPTEDITORWIDGET_H
