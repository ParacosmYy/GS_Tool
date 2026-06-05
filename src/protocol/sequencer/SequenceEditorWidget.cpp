/**
 * @file SequenceEditorWidget.cpp
 * @brief 协议序列编辑器面板 -- UI构建、事件处理、编辑逻辑
 *
 * 从 SequenceEditorWidget.h 拆分出的主实现，职责:
 *   1. setupUI -- 构建工具栏 + 步骤表格 + 详情面板三段式布局
 *   2. 工具栏动作(加载/保存/运行/停止)
 *   3. 步骤表格 CRUD + 拖拽排序
 *   4. 详情面板 ↔ 表格双向同步
 *   5. 引擎信号反馈(步骤高亮、状态栏更新)
 */

#include "protocol/sequencer/SequenceEditorWidget.h"
#include "protocol/sequencer/ProtocolSequencer.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QGroupBox>
#include <QFormLayout>

// ============================================================================
// 构造 / 析构
// ============================================================================

SequenceEditorWidget::SequenceEditorWidget(QWidget* parent)
    : QWidget(parent)
    , m_sequencer(new ProtocolSequencer(this))
{
    setObjectName("sequenceEditorWidget");
    setupUI();

    /* 引擎信号 */
    connect(m_sequencer, &ProtocolSequencer::stepStarted,
            this, &SequenceEditorWidget::onStepStarted);
    connect(m_sequencer, &ProtocolSequencer::stepCompleted,
            this, &SequenceEditorWidget::onStepCompleted);
    connect(m_sequencer, &ProtocolSequencer::sequenceComplete,
            this, &SequenceEditorWidget::onSequenceComplete);
}

SequenceEditorWidget::~SequenceEditorWidget()
{
}

// ============================================================================
// 公开方法
// ============================================================================

ProtocolSequencer* SequenceEditorWidget::sequencer() const { return m_sequencer; }

void SequenceEditorWidget::loadSteps(const QList<SequenceStep>& steps)
{
    m_sequencer->setSequence(steps);
    refreshTable();
    emit sequenceChanged();
}

QList<SequenceStep> SequenceEditorWidget::steps() const
{
    return m_sequencer->sequence();
}

// ============================================================================
// UI 构建
// ============================================================================

void SequenceEditorWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    /* 工具栏 */
    mainLayout->addWidget(createToolbar());

    /* 分割器: 表格 + 详情 */
    auto* splitter = new QSplitter(Qt::Vertical, this);
    splitter->setObjectName("seqSplitter");

    /* 步骤表格 */
    m_stepTable = new QTableWidget(splitter);
    m_stepTable->setObjectName("seqStepTable");
    m_stepTable->setColumnCount(5);
    m_stepTable->setHorizontalHeaderLabels({
        tr("#"), tr("Type"), tr("Description"), tr("Data"), tr("Delay (ms)")
    });
    m_stepTable->horizontalHeader()->setStretchLastSection(true);
    m_stepTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_stepTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_stepTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_stepTable->setDragEnabled(true);
    m_stepTable->setAcceptDrops(true);
    m_stepTable->setDragDropMode(QAbstractItemView::InternalMove);
    m_stepTable->setDefaultDropAction(Qt::MoveAction);
    connect(m_stepTable, &QTableWidget::cellChanged,
            this, &SequenceEditorWidget::onStepCellChanged);
    connect(m_stepTable, &QTableWidget::currentCellChanged,
            this, &SequenceEditorWidget::onStepSelectionChanged);

    /* 详情面板 */
    splitter->addWidget(createDetailPanel());
    splitter->setSizes({300, 200});

    mainLayout->addWidget(splitter);

    /* 状态标签 */
    m_lblStatus = new QLabel(tr("Ready"), this);
    m_lblStatus->setObjectName("seqStatusLabel");
    mainLayout->addWidget(m_lblStatus);
}

QWidget* SequenceEditorWidget::createToolbar()
{
    auto* toolbar = new QWidget(this);
    toolbar->setObjectName("seqToolbar");
    auto* layout = new QHBoxLayout(toolbar);
    layout->setContentsMargins(0, 0, 0, 0);

    m_btnLoad = new QPushButton(tr("Load"), toolbar);
    m_btnSave = new QPushButton(tr("Save"), toolbar);
    m_btnRun = new QPushButton(tr("Run"), toolbar);
    m_btnStop = new QPushButton(tr("Stop"), toolbar);
    m_btnPause = new QPushButton(tr("Pause"), toolbar);
    m_btnResume = new QPushButton(tr("Resume"), toolbar);

    m_btnLoad->setObjectName("seqBtnLoad");
    m_btnSave->setObjectName("seqBtnSave");
    m_btnRun->setObjectName("seqBtnRun");
    m_btnStop->setObjectName("seqBtnStop");
    m_btnPause->setObjectName("seqBtnPause");
    m_btnResume->setObjectName("seqBtnResume");

    auto* btnAdd = new QPushButton(tr("Add"), toolbar);
    auto* btnRemove = new QPushButton(tr("Remove"), toolbar);
    auto* btnUp = new QPushButton(tr("Up"), toolbar);
    auto* btnDown = new QPushButton(tr("Down"), toolbar);
    btnAdd->setObjectName("seqBtnAdd");
    btnRemove->setObjectName("seqBtnRemove");
    btnUp->setObjectName("seqBtnUp");
    btnDown->setObjectName("seqBtnDown");

    layout->addWidget(m_btnLoad);
    layout->addWidget(m_btnSave);
    layout->addSpacing(12);
    layout->addWidget(btnAdd);
    layout->addWidget(btnRemove);
    layout->addWidget(btnUp);
    layout->addWidget(btnDown);
    layout->addSpacing(12);
    layout->addWidget(m_btnRun);
    layout->addWidget(m_btnStop);
    layout->addWidget(m_btnPause);
    layout->addWidget(m_btnResume);
    layout->addStretch();

    connect(m_btnLoad, &QPushButton::clicked, this, &SequenceEditorWidget::onLoadClicked);
    connect(m_btnSave, &QPushButton::clicked, this, &SequenceEditorWidget::onSaveClicked);
    connect(m_btnRun, &QPushButton::clicked, this, &SequenceEditorWidget::onRunClicked);
    connect(m_btnStop, &QPushButton::clicked, this, &SequenceEditorWidget::onStopClicked);
    connect(m_btnPause, &QPushButton::clicked, this, &SequenceEditorWidget::onPauseClicked);
    connect(m_btnResume, &QPushButton::clicked, this, &SequenceEditorWidget::onResumeClicked);
    connect(btnAdd, &QPushButton::clicked, this, &SequenceEditorWidget::onAddStep);
    connect(btnRemove, &QPushButton::clicked, this, &SequenceEditorWidget::onRemoveStep);
    connect(btnUp, &QPushButton::clicked, this, &SequenceEditorWidget::onMoveUp);
    connect(btnDown, &QPushButton::clicked, this, &SequenceEditorWidget::onMoveDown);

    return toolbar;
}

QWidget* SequenceEditorWidget::createDetailPanel()
{
    auto* group = new QGroupBox(tr("Step Details"), this);
    group->setObjectName("seqDetailGroup");
    auto* form = new QFormLayout(group);

    m_detailType = new QComboBox(group);
    m_detailType->setObjectName("seqDetailType");
    m_detailType->addItem(tr("Send"), static_cast<int>(StepType::Send));
    m_detailType->addItem(tr("Receive"), static_cast<int>(StepType::Receive));
    m_detailType->addItem(tr("Delay"), static_cast<int>(StepType::Delay));
    m_detailType->addItem(tr("Wait For"), static_cast<int>(StepType::WaitFor));
    m_detailType->addItem(tr("Check"), static_cast<int>(StepType::Check));
    m_detailType->addItem(tr("Loop"), static_cast<int>(StepType::Loop));
    m_detailType->addItem(tr("Branch"), static_cast<int>(StepType::Branch));

    m_detailData = new QLineEdit(group);
    m_detailData->setObjectName("seqDetailData");
    m_detailData->setPlaceholderText(tr("HEX data (e.g. AA BB CC)"));

    m_detailDelay = new QSpinBox(group);
    m_detailDelay->setObjectName("seqDetailDelay");
    m_detailDelay->setRange(0, 600000);
    m_detailDelay->setSuffix(" ms");

    m_detailDesc = new QLineEdit(group);
    m_detailDesc->setObjectName("seqDetailDesc");
    m_detailDesc->setPlaceholderText(tr("Step description"));

    m_detailPattern = new QLineEdit(group);
    m_detailPattern->setObjectName("seqDetailPattern");
    m_detailPattern->setPlaceholderText(tr("Expected pattern (HEX or text)"));

    m_detailLoopCount = new QSpinBox(group);
    m_detailLoopCount->setObjectName("seqDetailLoopCount");
    m_detailLoopCount->setRange(1, 10000);

    form->addRow(tr("Type:"), m_detailType);
    form->addRow(tr("Data (HEX):"), m_detailData);
    form->addRow(tr("Delay:"), m_detailDelay);
    form->addRow(tr("Description:"), m_detailDesc);
    form->addRow(tr("Pattern:"), m_detailPattern);
    form->addRow(tr("Loop Count:"), m_detailLoopCount);

    connect(m_detailType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SequenceEditorWidget::onDetailTypeChanged);
    connect(m_detailData, &QLineEdit::textChanged,
            this, &SequenceEditorWidget::onDetailDataChanged);
    connect(m_detailDelay, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SequenceEditorWidget::onDetailDelayChanged);
    connect(m_detailDesc, &QLineEdit::textChanged,
            this, &SequenceEditorWidget::onDetailDescriptionChanged);
    connect(m_detailPattern, &QLineEdit::textChanged,
            this, &SequenceEditorWidget::onDetailPatternChanged);

    return group;
}

// ============================================================================
// 工具栏动作
// ============================================================================

void SequenceEditorWidget::onLoadClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Load Sequence"),
        QString(), tr("JSON Files (*.json)"));
    if (path.isEmpty()) { return; }
    if (m_sequencer->loadFromFile(path)) {
        refreshTable();
        emit sequenceChanged();
        m_lblStatus->setText(tr("Loaded: %1").arg(path));
    } else {
        QMessageBox::warning(this, tr("Load Error"), tr("Failed to load sequence file."));
    }
}

void SequenceEditorWidget::onSaveClicked()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save Sequence"),
        QString(), tr("JSON Files (*.json)"));
    if (path.isEmpty()) { return; }
    if (m_sequencer->saveToFile(path)) {
        m_lblStatus->setText(tr("Saved: %1").arg(path));
    } else {
        QMessageBox::warning(this, tr("Save Error"), tr("Failed to save sequence file."));
    }
}

void SequenceEditorWidget::onRunClicked()
{
    syncDetailToRow();
    m_sequencer->setSequence(steps());
    m_sequencer->execute();
    m_lblStatus->setText(tr("Running..."));
}

void SequenceEditorWidget::onStopClicked() { m_sequencer->stop(); }
void SequenceEditorWidget::onPauseClicked() { m_sequencer->pause(); }
void SequenceEditorWidget::onResumeClicked() { m_sequencer->resume(); }

// ============================================================================
// 步骤编辑
// ============================================================================

void SequenceEditorWidget::onAddStep()
{
    SequenceStep step;
    step.type = StepType::Send;
    step.description = tr("New step");
    auto current = m_sequencer->sequence();
    current.append(step);
    m_sequencer->setSequence(current);
    refreshTable();
    emit sequenceChanged();
}

void SequenceEditorWidget::onRemoveStep()
{
    int row = m_stepTable->currentRow();
    if (row < 0) { return; }
    auto current = m_sequencer->sequence();
    current.removeAt(row);
    m_sequencer->setSequence(current);
    refreshTable();
    emit sequenceChanged();
}

void SequenceEditorWidget::onMoveUp()
{
    int row = m_stepTable->currentRow();
    if (row <= 0) { return; }
    auto current = m_sequencer->sequence();
    std::swap(current[row], current[row - 1]);
    m_sequencer->setSequence(current);
    refreshTable();
    m_stepTable->selectRow(row - 1);
    emit sequenceChanged();
}

void SequenceEditorWidget::onMoveDown()
{
    int row = m_stepTable->currentRow();
    auto current = m_sequencer->sequence();
    if (row < 0 || row >= current.size() - 1) { return; }
    std::swap(current[row], current[row + 1]);
    m_sequencer->setSequence(current);
    refreshTable();
    m_stepTable->selectRow(row + 1);
    emit sequenceChanged();
}

void SequenceEditorWidget::onStepSelectionChanged()
{
    int row = m_stepTable->currentRow();
    if (row >= 0) {
        updateDetailPanel(row);
    } else {
        clearDetailPanel();
    }
}

void SequenceEditorWidget::onStepCellChanged(int row, int col)
{
    if (m_updatingFromDetail) { return; }
    auto current = m_sequencer->sequence();
    if (row < 0 || row >= current.size()) { return; }

    auto item = m_stepTable->item(row, col);
    if (!item) { return; }

    switch (col) {
    case 1: {
        QString txt = item->text().toLower();
        if (txt == "receive")      current[row].type = StepType::Receive;
        else if (txt == "delay")   current[row].type = StepType::Delay;
        else if (txt == "waitfor") current[row].type = StepType::WaitFor;
        else if (txt == "check")   current[row].type = StepType::Check;
        else if (txt == "loop")    current[row].type = StepType::Loop;
        else if (txt == "branch")  current[row].type = StepType::Branch;
        else                       current[row].type = StepType::Send;
        break;
    }
    case 2: current[row].description = item->text(); break;
    case 3: current[row].data = QByteArray::fromHex(item->text().toUtf8()); break;
    case 4: current[row].delayMs = item->text().toInt(); break;
    default: return;
    }

    m_sequencer->setSequence(current);
    emit sequenceChanged();
}

// ============================================================================
// 详情面板回调
// ============================================================================

void SequenceEditorWidget::onDetailTypeChanged(int index)
{
    Q_UNUSED(index)
    syncDetailToRow();
}

void SequenceEditorWidget::onDetailDataChanged() { syncDetailToRow(); }
void SequenceEditorWidget::onDetailDelayChanged(int value) { Q_UNUSED(value); syncDetailToRow(); }
void SequenceEditorWidget::onDetailDescriptionChanged(const QString&) { syncDetailToRow(); }
void SequenceEditorWidget::onDetailPatternChanged(const QString&) { syncDetailToRow(); }

// ============================================================================
// 引擎回调
// ============================================================================

void SequenceEditorWidget::onStepStarted(int index)
{
    if (index >= 0 && index < m_stepTable->rowCount()) {
        m_stepTable->selectRow(index);
    }
}

void SequenceEditorWidget::onStepCompleted(int index, bool success)
{
    Q_UNUSED(index)
    Q_UNUSED(success)
}

void SequenceEditorWidget::onSequenceComplete(const SequenceResult& result)
{
    QString msg = result.success
        ? tr("Sequence completed: %1 steps in %2 ms")
            .arg(result.stepsCompleted).arg(result.durationMs)
        : tr("Sequence failed: %1 (step %2)")
            .arg(result.errorMessage).arg(result.stepsCompleted);
    m_lblStatus->setText(msg);
}

// ============================================================================
// 表格操作
// ============================================================================

void SequenceEditorWidget::refreshTable()
{
    m_stepTable->blockSignals(true);
    auto seq = m_sequencer->sequence();
    m_stepTable->setRowCount(seq.size());
    for (int i = 0; i < seq.size(); ++i) {
        populateRow(i, seq[i]);
    }
    m_stepTable->blockSignals(false);
}

void SequenceEditorWidget::populateRow(int row, const SequenceStep& step)
{
    auto* itemIdx = new QTableWidgetItem(QString::number(row + 1));
    itemIdx->setFlags(itemIdx->flags() & ~Qt::ItemIsEditable);
    m_stepTable->setItem(row, 0, itemIdx);

    m_stepTable->setItem(row, 1, new QTableWidgetItem(stepTypeDisplayName(step.type)));
    m_stepTable->setItem(row, 2, new QTableWidgetItem(step.description));
    m_stepTable->setItem(row, 3, new QTableWidgetItem(QString(step.data.toHex(' '))));
    m_stepTable->setItem(row, 4, new QTableWidgetItem(QString::number(step.delayMs)));
}

void SequenceEditorWidget::updateDetailPanel(int row)
{
    auto seq = m_sequencer->sequence();
    if (row < 0 || row >= seq.size()) { return; }

    m_updatingFromDetail = true;
    const auto& step = seq[row];
    m_detailType->setCurrentIndex(static_cast<int>(step.type));
    m_detailData->setText(QString(step.data.toHex(' ')));
    m_detailDelay->setValue(step.delayMs);
    m_detailDesc->setText(step.description);
    m_detailPattern->setText(step.expectedPattern);
    m_detailLoopCount->setValue(step.loopCount);
    m_updatingFromDetail = false;
}

void SequenceEditorWidget::clearDetailPanel()
{
    m_updatingFromDetail = true;
    m_detailType->setCurrentIndex(0);
    m_detailData->clear();
    m_detailDelay->setValue(0);
    m_detailDesc->clear();
    m_detailPattern->clear();
    m_detailLoopCount->setValue(1);
    m_updatingFromDetail = false;
}

void SequenceEditorWidget::syncDetailToRow()
{
    if (m_updatingFromDetail) { return; }
    int row = m_stepTable->currentRow();
    if (row < 0) { return; }

    auto current = m_sequencer->sequence();
    if (row >= current.size()) { return; }

    auto& step = current[row];
    step.type = static_cast<StepType>(m_detailType->currentData().toInt());
    step.data = QByteArray::fromHex(m_detailData->text().toUtf8());
    step.delayMs = m_detailDelay->value();
    step.description = m_detailDesc->text();
    step.expectedPattern = m_detailPattern->text();
    step.loopCount = m_detailLoopCount->value();

    m_sequencer->setSequence(current);
    m_stepTable->blockSignals(true);
    populateRow(row, step);
    m_stepTable->blockSignals(false);
    emit sequenceChanged();
}

QString SequenceEditorWidget::stepTypeDisplayName(StepType type)
{
    switch (type) {
    case StepType::Send:    return tr("Send");
    case StepType::Receive: return tr("Receive");
    case StepType::Delay:   return tr("Delay");
    case StepType::WaitFor: return tr("WaitFor");
    case StepType::Check:   return tr("Check");
    case StepType::Loop:    return tr("Loop");
    case StepType::Branch:  return tr("Branch");
    }
    return tr("Send");
}
