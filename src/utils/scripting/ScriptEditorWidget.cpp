/**
 * @file ScriptEditorWidget.cpp
 * @brief 脚本编辑器面板实现 — UI 构建、信号连接、脚本管理、执行调度
 *
 * 布局: 左侧脚本列表+增删 | 右侧代码编辑器+语言选择+参数表+运行/停止+输出控制台。
 * 统计见 ScriptEditorWidgetStats.cpp。
 */

#include "utils/scripting/ScriptEditorWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QFontDatabase>
#include <QHeaderView>
#include <QDateTime>
#include <QScrollBar>

// ============================================================
// 构造 + UI
// ============================================================

/** @brief 构造脚本编辑器面板，初始化 UI 布局和信号连接 */
ScriptEditorWidget::ScriptEditorWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("ScriptEditorWidget"));
    setupUi();
    connectSignals();
}

/** @brief 构建 UI: 左右分栏(脚本列表 | 编辑器+控制台) */
void ScriptEditorWidget::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setObjectName(QStringLiteral("scriptMainSplitter"));

    // ---- 左侧: 脚本列表 + 增删按钮 ----
    auto* leftWidget = new QWidget(m_mainSplitter);
    leftWidget->setObjectName(QStringLiteral("scriptLeftPanel"));
    auto* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(4);

    auto* listLabel = new QLabel(tr("脚本列表"), leftWidget);
    listLabel->setObjectName(QStringLiteral("scriptListLabel"));
    leftLayout->addWidget(listLabel);

    m_scriptList = new QListWidget(leftWidget);
    m_scriptList->setObjectName(QStringLiteral("scriptListWidget"));
    leftLayout->addWidget(m_scriptList);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(4);
    m_addBtn = new QPushButton(tr("添加"), leftWidget);
    m_addBtn->setObjectName(QStringLiteral("scriptAddBtn"));
    m_removeBtn = new QPushButton(tr("删除"), leftWidget);
    m_removeBtn->setObjectName(QStringLiteral("scriptRemoveBtn"));
    m_removeBtn->setEnabled(false);
    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_removeBtn);
    leftLayout->addLayout(btnLayout);

    // ---- 右侧: 编辑器 + 参数 + 按钮 + 控制台 ----
    auto* rightWidget = new QWidget(m_mainSplitter);
    rightWidget->setObjectName(QStringLiteral("scriptRightPanel"));
    auto* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(4);

    // 语言选择行
    auto* langLayout = new QHBoxLayout();
    langLayout->setSpacing(4);
    auto* langLabel = new QLabel(tr("语言:"), rightWidget);
    langLabel->setObjectName(QStringLiteral("scriptLangLabel"));
    m_languageCombo = new QComboBox(rightWidget);
    m_languageCombo->setObjectName(QStringLiteral("scriptLanguageCombo"));
    m_languageCombo->addItem(tr("JavaScript"), static_cast<int>(ScriptLanguage::JavaScript));
    m_languageCombo->addItem(tr("Python (预留)"), static_cast<int>(ScriptLanguage::Python));
    langLayout->addWidget(langLabel);
    langLayout->addWidget(m_languageCombo);
    langLayout->addStretch();
    rightLayout->addLayout(langLayout);

    // 代码编辑器
    m_codeEditor = new QPlainTextEdit(rightWidget);
    m_codeEditor->setObjectName(QStringLiteral("scriptCodeEditor"));
    QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    monoFont.setPointSize(10);
    m_codeEditor->setFont(monoFont);
    m_codeEditor->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_codeEditor->setPlaceholderText(tr("// 在此输入脚本代码...\n// 可用函数: serialRead(), serialWrite(hex), log(msg)"));
    rightLayout->addWidget(m_codeEditor, 3);

    // 参数表
    auto* paramLabel = new QLabel(tr("参数 (${key} 替换):"), rightWidget);
    paramLabel->setObjectName(QStringLiteral("scriptParamLabel"));
    rightLayout->addWidget(paramLabel);

    m_paramTable = new QTableWidget(0, 2, rightWidget);
    m_paramTable->setObjectName(QStringLiteral("scriptParamTable"));
    m_paramTable->setHorizontalHeaderLabels({tr("参数名"), tr("默认值")});
    m_paramTable->horizontalHeader()->setStretchLastSection(true);
    m_paramTable->setMaximumHeight(100);
    rightLayout->addWidget(m_paramTable);

    // 运行/停止按钮行
    auto* runLayout = new QHBoxLayout();
    runLayout->setSpacing(4);
    m_runBtn = new QPushButton(tr("运行"), rightWidget);
    m_runBtn->setObjectName(QStringLiteral("scriptRunBtn"));
    m_stopBtn = new QPushButton(tr("停止"), rightWidget);
    m_stopBtn->setObjectName(QStringLiteral("scriptStopBtn"));
    m_stopBtn->setEnabled(false);
    m_statusLabel = new QLabel(tr("就绪"), rightWidget);
    m_statusLabel->setObjectName(QStringLiteral("scriptStatusLabel"));
    runLayout->addWidget(m_runBtn);
    runLayout->addWidget(m_stopBtn);
    runLayout->addStretch();
    runLayout->addWidget(m_statusLabel);
    rightLayout->addLayout(runLayout);

    // 输出控制台
    auto* outLabel = new QLabel(tr("输出:"), rightWidget);
    outLabel->setObjectName(QStringLiteral("scriptOutputLabel"));
    rightLayout->addWidget(outLabel);

    m_outputConsole = new QPlainTextEdit(rightWidget);
    m_outputConsole->setObjectName(QStringLiteral("scriptOutputConsole"));
    m_outputConsole->setFont(monoFont);
    m_outputConsole->setReadOnly(true);
    m_outputConsole->setMaximumHeight(150);
    m_outputConsole->setPlaceholderText(tr("脚本输出将显示在此处..."));
    rightLayout->addWidget(m_outputConsole, 1);

    // 组装分栏
    m_mainSplitter->addWidget(leftWidget);
    m_mainSplitter->addWidget(rightWidget);
    m_mainSplitter->setSizes({200, 600});
    mainLayout->addWidget(m_mainSplitter);
}

// ============================================================
// 信号连接
// ============================================================

/** @brief 连接所有内部信号槽 */
void ScriptEditorWidget::connectSignals()
{
    connect(m_addBtn, &QPushButton::clicked, this, &ScriptEditorWidget::onAddScript);
    connect(m_removeBtn, &QPushButton::clicked, this, &ScriptEditorWidget::onRemoveScript);
    connect(m_runBtn, &QPushButton::clicked, this, &ScriptEditorWidget::onRunClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, &ScriptEditorWidget::onStopClicked);
    connect(m_scriptList, &QListWidget::currentRowChanged,
            this, &ScriptEditorWidget::onScriptSelected);
    connect(m_codeEditor, &QPlainTextEdit::textChanged,
            this, &ScriptEditorWidget::onCodeChanged);
}

// ============================================================
// 引擎设置
// ============================================================

void ScriptEditorWidget::setEngine(ScriptEngine* engine)
{
    if (m_engine) {
        disconnect(m_engine, nullptr, this, nullptr);
    }
    m_engine = engine;
    if (m_engine) {
        connect(m_engine, &ScriptEngine::scriptExecuted,
                this, &ScriptEditorWidget::onScriptExecuted);
        connect(m_engine, &ScriptEngine::outputReady,
                this, &ScriptEditorWidget::onOutputReady);
        connect(m_engine, &ScriptEngine::errorOccurred,
                this, &ScriptEditorWidget::onErrorOccurred);
        connect(m_engine, &ScriptEngine::sendDataRequested,
                this, &ScriptEditorWidget::sendDataRequested);
        updateScriptList();
    }
}

// ============================================================
// 公共查询
// ============================================================

QString ScriptEditorWidget::currentCode() const { return m_codeEditor->toPlainText(); }
int ScriptEditorWidget::currentScriptIndex() const { return m_currentIndex; }

// ============================================================
// 槽函数
// ============================================================

/** @brief 运行当前编辑器中的脚本代码 */
void ScriptEditorWidget::onRunClicked()
{
    if (!m_engine) {
        appendOutput(tr("[错误] 脚本引擎未初始化"), QStringLiteral("#FF4444"));
        return;
    }

    saveCurrentScript();
    QString code = m_codeEditor->toPlainText();
    if (code.trimmed().isEmpty()) {
        appendOutput(tr("[警告] 代码为空"), QStringLiteral("#FFAA00"));
        return;
    }

    ++m_totalRuns;
    m_statusLabel->setText(tr("执行中..."));
    m_runBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    m_outputConsole->clear();

    /* 收集参数 */
    QMap<QString, QString> params;
    for (int i = 0; i < m_paramTable->rowCount(); ++i) {
        auto* keyItem = m_paramTable->item(i, 0);
        auto* valItem = m_paramTable->item(i, 1);
        if (keyItem && !keyItem->text().trimmed().isEmpty()) {
            params[keyItem->text().trimmed()] = valItem ? valItem->text() : QString();
        }
    }

    ScriptAction action;
    action.name = m_currentIndex >= 0 ? m_scriptList->item(m_currentIndex)->text()
                                      : tr("临时脚本");
    action.code = code;
    action.language = static_cast<ScriptLanguage>(
        m_languageCombo->currentData().toInt());
    action.params = params;
    action.enabled = true;

    m_engine->execute(action);
}

/** @brief 停止执行(预留 — QJSEngine 同步执行无法中断) */
void ScriptEditorWidget::onStopClicked()
{
    m_statusLabel->setText(tr("已停止"));
    m_runBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
}

/** @brief 添加新脚本到列表 */
void ScriptEditorWidget::onAddScript()
{
    ScriptAction action;
    action.name = tr("新脚本 %1").arg(m_scriptList->count() + 1);
    action.language = ScriptLanguage::JavaScript;
    action.code = QStringLiteral("// %1\n").arg(action.name);
    action.enabled = true;

    if (m_engine) {
        m_engine->addScript(action);
    }

    ++m_totalScriptsAdded;
    updateScriptList();
    m_scriptList->setCurrentRow(m_scriptList->count() - 1);
}

/** @brief 删除当前选中的脚本 */
void ScriptEditorWidget::onRemoveScript()
{
    int row = m_scriptList->currentRow();
    if (row < 0) return;

    if (m_engine) {
        m_engine->removeScript(row);
    }
    ++m_totalScriptsRemoved;
    m_currentIndex = -1;
    updateScriptList();
}

/** @brief 脚本列表选中切换 — 保存旧脚本并加载新脚本 */
void ScriptEditorWidget::onScriptSelected(int row)
{
    saveCurrentScript();
    loadScriptToEditor(row);
    m_removeBtn->setEnabled(row >= 0);
}

/** @brief 代码编辑器内容变更 — 延迟保存 */
void ScriptEditorWidget::onCodeChanged()
{
    if (m_updatingFromCode) return;
    /* 标记当前脚本已修改(不立即保存，避免频繁写) */
    m_statusLabel->setText(tr("已修改"));
}

/** @brief 脚本执行结果回调 */
void ScriptEditorWidget::onScriptExecuted(const ScriptResult& result)
{
    m_runBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);

    if (result.success) {
        ++m_totalSuccessRuns;
        m_statusLabel->setText(tr("完成 (%1ms)").arg(result.durationMs));
        if (!result.output.isEmpty()) {
            appendOutput(result.output);
        }
        if (!result.returnValue.isNull()) {
            appendOutput(tr("返回值: %1").arg(result.returnValue.toString()),
                         QStringLiteral("#4488FF"));
        }
    } else {
        ++m_totalFailedRuns;
        m_statusLabel->setText(tr("失败 (行%1)").arg(result.line));
        appendOutput(tr("[错误] %1 (行 %2)").arg(result.error).arg(result.line),
                     QStringLiteral("#FF4444"));
    }
}

void ScriptEditorWidget::onOutputReady(const QString& text)
{
    appendOutput(text);
}

void ScriptEditorWidget::onErrorOccurred(const QString& message)
{
    appendOutput(tr("[错误] %1").arg(message), QStringLiteral("#FF4444"));
}

// ============================================================
// 脚本管理辅助
// ============================================================

/** @brief 刷新脚本列表显示 */
void ScriptEditorWidget::updateScriptList()
{
    m_scriptList->clear();
    if (!m_engine) return;

    for (const auto& script : m_engine->scripts()) {
        auto* item = new QListWidgetItem(
            script.enabled ? script.name : QStringLiteral("[%1]").arg(script.name));
        item->setData(Qt::UserRole, script.enabled);
        m_scriptList->addItem(item);
    }
}

/** @brief 保存当前编辑器内容到引擎脚本列表 */
void ScriptEditorWidget::saveCurrentScript()
{
    if (!m_engine || m_currentIndex < 0 ||
        m_currentIndex >= m_engine->scripts().size()) return;

    ScriptAction action = m_engine->scripts().at(m_currentIndex);
    action.code = m_codeEditor->toPlainText();
    action.language = static_cast<ScriptLanguage>(
        m_languageCombo->currentData().toInt());

    /* 收集参数 */
    action.params.clear();
    for (int i = 0; i < m_paramTable->rowCount(); ++i) {
        auto* keyItem = m_paramTable->item(i, 0);
        auto* valItem = m_paramTable->item(i, 1);
        if (keyItem && !keyItem->text().trimmed().isEmpty()) {
            action.params[keyItem->text().trimmed()] = valItem ? valItem->text() : QString();
        }
    }
    m_engine->updateScript(m_currentIndex, action);
}

/** @brief 加载指定索引的脚本到编辑器和参数表 */
void ScriptEditorWidget::loadScriptToEditor(int index)
{
    m_updatingFromCode = true;
    m_currentIndex = index;
    m_paramTable->setRowCount(0);

    if (!m_engine || index < 0 || index >= m_engine->scripts().size()) {
        m_codeEditor->clear();
        m_updatingFromCode = false;
        return;
    }

    const auto& action = m_engine->scripts().at(index);
    m_codeEditor->setPlainText(action.code);

    /* 设置语言选择 */
    for (int i = 0; i < m_languageCombo->count(); ++i) {
        if (m_languageCombo->itemData(i).toInt() == static_cast<int>(action.language)) {
            m_languageCombo->setCurrentIndex(i);
            break;
        }
    }

    /* 填充参数表 */
    int row = 0;
    for (auto it = action.params.constBegin(); it != action.params.constEnd(); ++it) {
        m_paramTable->insertRow(row);
        m_paramTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        m_paramTable->setItem(row, 1, new QTableWidgetItem(it.value()));
        ++row;
    }
    /* 始终保留一个空行便于添加新参数 */
    m_paramTable->insertRow(row);

    m_statusLabel->setText(tr("就绪"));
    m_updatingFromCode = false;
}

/** @brief 向输出控制台追加带时间戳和颜色的文本 */
void ScriptEditorWidget::appendOutput(const QString& text, const QString& color)
{
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz"));
    QString html;
    if (color.isEmpty()) {
        html = QStringLiteral("<span style='color:gray'>[%1]</span> %2")
                   .arg(timestamp, text.toHtmlEscaped());
    } else {
        html = QStringLiteral("<span style='color:gray'>[%1]</span> "
                              "<span style='color:%2'>%3</span>")
                   .arg(timestamp, color, text.toHtmlEscaped());
    }
    m_outputConsole->appendHtml(html);
    m_outputConsole->verticalScrollBar()->setValue(
        m_outputConsole->verticalScrollBar()->maximum());
}

// 统计见 ScriptEditorWidgetStats.cpp
