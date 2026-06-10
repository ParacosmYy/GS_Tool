/**
 * @file RegisterMapEditor.h
 * @brief 寄存器地图编辑器 -- 可视化查看/编辑设备寄存器地图
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 包含工具栏（加载/保存/导出JSON）、寄存器表格、搜索栏和位域查看面板。
 * 统计查询方法见：@see RegisterMapEditorStats.cpp
 */

#ifndef REGISTERMAPEDITOR_H
#define REGISTERMAPEDITOR_H

#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTableView>
#include <QLabel>
#include <QTextEdit>
#include <QWidget>

#include "utils/regmap/RegisterMapModel.h"

/**
 * @class RegisterMapEditor
 * @brief 寄存器地图交互式编辑器控件
 *
 * 典型用法：加载 JSON 格式的寄存器地图文件，在表格中查看和编辑寄存器值，
 * 选中某行后底部面板显示该寄存器的位域分解。
 */
class RegisterMapEditor : public QWidget {
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalFileLoads = 0;     ///< 文件加载次数
        quint64 totalFileSaves = 0;     ///< 文件保存次数
        quint64 totalJsonExports = 0;   ///< JSON 导出次数
        quint64 totalRegisterEdits = 0; ///< 寄存器编辑次数
        int largestMapLoaded = 0;       ///< 最大加载的地图寄存器数
    };

    /**
     * @brief 构造函数
     * @param parent 父控件
     */
    explicit RegisterMapEditor(QWidget *parent = nullptr);

    /** @brief 从 JSON 文件加载寄存器地图 @param filePath JSON 文件路径 @return 成功与否 */
    bool loadFromJson(const QString &filePath);

    /** @brief 保存寄存器地图到 JSON 文件 @param filePath 目标路径 @return 成功与否 */
    bool saveToJson(const QString &filePath);

    /** @brief 导出当前值到 JSON @param filePath 目标路径 @return 成功与否 */
    bool exportToJson(const QString &filePath);

    /** @brief 获取模型指针 @return RegisterMapModel 指针 */
    RegisterMapModel *model() const;

    /** @brief 获取统计信息 @return 统计快照常引用 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 文件加载完成信号 @param count 寄存器数量 */
    void fileLoaded(int count);

    /** @brief 文件保存完成信号 @param filePath 文件路径 */
    void fileSaved(const QString &filePath);

private slots:
    void onLoadClicked();           ///< 加载按钮点击
    void onSaveClicked();           ///< 保存按钮点击
    void onExportClicked();         ///< 导出按钮点击
    void onSearchTextChanged(const QString &text); ///< 搜索文本变更
    void onTableRowClicked(const QModelIndex &index); ///< 表格行点击
    void onRegisterValueChanged(int row, quint64 val); ///< 值变更

private:
    void setupUI();                 ///< 初始化界面
    void updateBitFieldPanel(const RegisterEntry &reg, quint64 value); ///< 更新位域面板

    // ---- UI 控件 ----
    QPushButton *m_loadBtn;         ///< 加载按钮
    QPushButton *m_saveBtn;         ///< 保存按钮
    QPushButton *m_exportBtn;       ///< 导出按钮
    QLineEdit *m_searchEdit;        ///< 搜索输入框
    QTableView *m_tableView;        ///< 寄存器表格
    QTextEdit *m_bitFieldPanel;     ///< 位域显示面板
    QSplitter *m_splitter;          ///< 上下分割器
    RegisterMapModel *m_model;      ///< 数据模型
    Stats m_stats;                  ///< 统计计数器
};

#endif // REGISTERMAPEDITOR_H
