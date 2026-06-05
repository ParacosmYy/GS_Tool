/**
 * @file SequenceEditorWidget.h
 * @brief 协议序列编辑器面板 -- 步骤列表编辑、详情面板、序列执行控制
 *
 * 提供 GUI 编辑界面:
 *   - 工具栏: 加载/保存/运行/停止/暂停/恢复
 *   - 步骤列表表格: #、Type、Description、Data、Delay
 *   - 步骤详情编辑器: 类型选择、数据编辑(hex)、延时设置
 *   - 增删步骤、上下移动、拖拽排序
 *   - objectName 标注所有 QWidget 实例
 *
 * 协作: ProtocolSequencer(执行引擎)
 */

#ifndef SEQUENCEEDITORWIDGET_H
#define SEQUENCEEDITORWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

#include "protocol/sequencer/SequencerTypes.h"

class ProtocolSequencer;

/**
 * @brief 协议序列编辑器面板
 *
 * 组合 ProtocolSequencer 引擎和编辑 UI，
 * 用户可加载/创建/编辑/执行协议序列。
 */
class SequenceEditorWidget : public QWidget {
    Q_OBJECT

public:
    //-- 构造/析构 --//
    explicit SequenceEditorWidget(QWidget* parent = nullptr);
    ~SequenceEditorWidget() override;

    /** @brief 获取内部序列器引擎(用于外部信号连接) */
    ProtocolSequencer* sequencer() const;

    //-- 序列操作 --//
    /** @brief 加载序列到编辑器和引擎 @param steps 步骤列表 */
    void loadSteps(const QList<SequenceStep>& steps);

    /** @brief 获取当前编辑器中的步骤列表 */
    QList<SequenceStep> steps() const;

    //-- 统计 --//
    const SequencerStats& stats() const;
    void resetStatistics();

signals:
    /** @brief 序列内容已变更 */
    void sequenceChanged();

private slots:
    //-- 工具栏动作 --//
    void onLoadClicked();
    void onSaveClicked();
    void onRunClicked();
    void onStopClicked();
    void onPauseClicked();
    void onResumeClicked();

    //-- 步骤编辑 --//
    void onAddStep();
    void onRemoveStep();
    void onMoveUp();
    void onMoveDown();
    void onStepSelectionChanged();
    void onStepCellChanged(int row, int col);
    void onDetailTypeChanged(int index);
    void onDetailDataChanged();
    void onDetailDelayChanged(int value);
    void onDetailDescriptionChanged(const QString& text);
    void onDetailPatternChanged(const QString& text);

    //-- 引擎回调 --//
    void onStepStarted(int index);
    void onStepCompleted(int index, bool success);
    void onSequenceComplete(const SequenceResult& result);

private:
    //-- UI 构建 --//
    void setupUI();
    QWidget* createToolbar();
    QWidget* createDetailPanel();

    //-- 内部辅助 --//
    void refreshTable();
    void populateRow(int row, const SequenceStep& step);
    SequenceStep rowToStep(int row) const;
    void syncDetailToRow();
    void updateDetailPanel(int row);
    void clearDetailPanel();
    static QString stepTypeDisplayName(StepType type);

    //-- 成员变量 --//
    ProtocolSequencer* m_sequencer = nullptr;   ///< 序列器引擎

    //-- 工具栏控件 --//
    QPushButton* m_btnLoad = nullptr;
    QPushButton* m_btnSave = nullptr;
    QPushButton* m_btnRun = nullptr;
    QPushButton* m_btnStop = nullptr;
    QPushButton* m_btnPause = nullptr;
    QPushButton* m_btnResume = nullptr;

    //-- 步骤表格 --//
    QTableWidget* m_stepTable = nullptr;

    //-- 详情面板控件 --//
    QComboBox*   m_detailType = nullptr;
    QLineEdit*   m_detailData = nullptr;
    QSpinBox*    m_detailDelay = nullptr;
    QLineEdit*   m_detailDesc = nullptr;
    QLineEdit*   m_detailPattern = nullptr;
    QSpinBox*    m_detailLoopCount = nullptr;
    QLabel*      m_lblStatus = nullptr;

    //-- 状态 --//
    bool m_updatingFromDetail = false;           ///< 防止循环更新标志
};

#endif // SEQUENCEEDITORWIDGET_H
