/**
 * @file DeviceSimulatorPanel.cpp
 * @brief 设备模拟器配置面板实现 -- UI构建、规则表管理、JSON导入导出
 */
#include "utils/simulator/DeviceSimulatorPanel.h"
#include "utils/simulator/DeviceSimulator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QComboBox>

// ── 构造 ──

DeviceSimulatorPanel::DeviceSimulatorPanel(
    QSharedPointer<DeviceSimulator> simulator, QWidget* parent)
    : QWidget(parent)
    , m_simulator(simulator)
{
    setupUI();
    if (m_simulator) loadFromSimulator();
}

// ── 公共方法 ──

void DeviceSimulatorPanel::setSimulator(QSharedPointer<DeviceSimulator> simulator)
{
    m_simulator = simulator;
    if (m_simulator) loadFromSimulator();
}

QSharedPointer<DeviceSimulator> DeviceSimulatorPanel::simulator() const
{
    return m_simulator;
}

// ── JSON 导入/导出 ──

bool DeviceSimulatorPanel::exportToJson(const QString& filePath) const
{
    QJsonArray rulesArr;
    for (int i = 0; i < m_table->rowCount(); ++i) {
        SimResponse r = const_cast<DeviceSimulatorPanel*>(this)->buildRuleFromRow(i);
        QJsonObject obj;
        obj["command"] = QString::fromUtf8(r.commandPattern.toHex());
        obj["fixedData"] = QString::fromUtf8(r.fixedData.toHex());
        obj["mode"] = static_cast<int>(r.mode);
        obj["strategy"] = static_cast<int>(r.strategy);
        obj["minDelayMs"] = r.minDelayMs;
        obj["maxDelayMs"] = r.maxDelayMs;
        obj["enabled"] = r.enabled;
        QJsonArray scripts;
        for (const auto& s : r.scriptedResponses)
            scripts.append(QString::fromUtf8(s.toHex()));
        obj["scriptedResponses"] = scripts;
        rulesArr.append(obj);
    }

    QJsonObject root;
    root["rules"] = rulesArr;
    root["autoEcho"] = m_echoCheck->isChecked();
    root["noiseRate"] = m_noiseSpin->value();
    root["defaultMinDelayMs"] = m_minDelaySpin->value();
    root["defaultMaxDelayMs"] = m_maxDelaySpin->value();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool DeviceSimulatorPanel::importFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    m_echoCheck->setChecked(root["autoEcho"].toBool(false));
    m_noiseSpin->setValue(root["noiseRate"].toDouble(0.0));
    m_minDelaySpin->setValue(root["defaultMinDelayMs"].toInt(10));
    m_maxDelaySpin->setValue(root["defaultMaxDelayMs"].toInt(50));

    QJsonArray rules = root["rules"].toArray();
    m_syncing = true;
    m_table->setRowCount(0);
    for (const QJsonValue& val : rules) {
        QJsonObject obj = val.toObject();
        SimResponse r;
        r.commandPattern = QByteArray::fromHex(obj["command"].toString().toUtf8());
        r.fixedData = QByteArray::fromHex(obj["fixedData"].toString().toUtf8());
        r.mode = static_cast<ResponseMode>(obj["mode"].toInt(0));
        r.strategy = static_cast<MatchStrategy>(obj["strategy"].toInt(0));
        r.minDelayMs = obj["minDelayMs"].toInt(10);
        r.maxDelayMs = obj["maxDelayMs"].toInt(50);
        r.enabled = obj["enabled"].toBool(true);
        QJsonArray scripts = obj["scriptedResponses"].toArray();
        for (const QJsonValue& sv : scripts)
            r.scriptedResponses.append(QByteArray::fromHex(sv.toString().toUtf8()));
        int row = m_table->rowCount();
        m_table->insertRow(row);
        fillRow(row, r);
    }
    m_syncing = false;
    syncToSimulator();
    emit rulesChanged();
    return true;
}

// ── Slots ──

void DeviceSimulatorPanel::onAddRule()
{
    SimResponse defaultRule;
    defaultRule.commandPattern = "CMD";
    defaultRule.fixedData = "OK";
    int row = m_table->rowCount();
    m_syncing = true;
    m_table->insertRow(row);
    fillRow(row, defaultRule);
    m_syncing = false;
    syncToSimulator();
    emit rulesChanged();
}

void DeviceSimulatorPanel::onRemoveRule()
{
    int row = m_table->currentRow();
    if (row < 0) return;
    m_syncing = true;
    m_table->removeRow(row);
    m_syncing = false;
    syncToSimulator();
    emit rulesChanged();
}

void DeviceSimulatorPanel::onClearRules()
{
    m_syncing = true;
    m_table->setRowCount(0);
    m_syncing = false;
    syncToSimulator();
    emit rulesChanged();
}

void DeviceSimulatorPanel::onExportClicked()
{
    QString path = QFileDialog::getSaveFileName(
        this, tr("导出模拟器规则"), QString(),
        tr("JSON 文件 (*.json)"));
    if (path.isEmpty()) return;
    if (!exportToJson(path))
        QMessageBox::warning(this, tr("导出失败"), tr("无法写入文件: %1").arg(path));
}

void DeviceSimulatorPanel::onImportClicked()
{
    QString path = QFileDialog::getOpenFileName(
        this, tr("导入模拟器规则"), QString(),
        tr("JSON 文件 (*.json)"));
    if (path.isEmpty()) return;
    if (!importFromJson(path))
        QMessageBox::warning(this, tr("导入失败"), tr("无法读取文件: %1").arg(path));
}

void DeviceSimulatorPanel::onCellChanged(int row, int col)
{
    Q_UNUSED(row)
    Q_UNUSED(col)
    if (m_syncing) return;
    syncToSimulator();
    emit rulesChanged();
}

// ── 私有方法 ──

void DeviceSimulatorPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    // 顶部全局配置栏
    auto* configLayout = new QHBoxLayout();
    configLayout->setSpacing(12);

    m_echoCheck = new QCheckBox(tr("自动回显"), this);
    m_echoCheck->setObjectName("simEchoCheck");
    configLayout->addWidget(m_echoCheck);

    configLayout->addWidget(new QLabel(tr("噪声率:"), this));
    m_noiseSpin = new QDoubleSpinBox(this);
    m_noiseSpin->setObjectName("simNoiseSpin");
    m_noiseSpin->setRange(0.0, 1.0);
    m_noiseSpin->setSingleStep(0.01);
    m_noiseSpin->setDecimals(2);
    configLayout->addWidget(m_noiseSpin);

    configLayout->addWidget(new QLabel(tr("最小延迟(ms):"), this));
    m_minDelaySpin = new QSpinBox(this);
    m_minDelaySpin->setObjectName("simMinDelaySpin");
    m_minDelaySpin->setRange(0, 60000);
    m_minDelaySpin->setValue(10);
    configLayout->addWidget(m_minDelaySpin);

    configLayout->addWidget(new QLabel(tr("最大延迟(ms):"), this));
    m_maxDelaySpin = new QSpinBox(this);
    m_maxDelaySpin->setObjectName("simMaxDelaySpin");
    m_maxDelaySpin->setRange(0, 60000);
    m_maxDelaySpin->setValue(50);
    configLayout->addWidget(m_maxDelaySpin);

    configLayout->addStretch();
    mainLayout->addLayout(configLayout);

    // 中部规则表
    m_table = new QTableWidget(0, 7, this);
    m_table->setObjectName("simRulesTable");
    m_table->setHorizontalHeaderLabels({
        tr("命令模式"), tr("匹配策略"), tr("响应模式"),
        tr("响应数据(HEX)"), tr("最小延迟"), tr("最大延迟"), tr("启用")
    });
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    mainLayout->addWidget(m_table);

    // 底部按钮栏
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(6);

    m_addBtn = new QPushButton(tr("添加规则"), this);
    m_addBtn->setObjectName("simAddBtn");
    btnLayout->addWidget(m_addBtn);

    m_removeBtn = new QPushButton(tr("删除规则"), this);
    m_removeBtn->setObjectName("simRemoveBtn");
    btnLayout->addWidget(m_removeBtn);

    m_clearBtn = new QPushButton(tr("清空全部"), this);
    m_clearBtn->setObjectName("simClearBtn");
    btnLayout->addWidget(m_clearBtn);

    btnLayout->addStretch();

    m_exportBtn = new QPushButton(tr("导出 JSON"), this);
    m_exportBtn->setObjectName("simExportBtn");
    btnLayout->addWidget(m_exportBtn);

    m_importBtn = new QPushButton(tr("导入 JSON"), this);
    m_importBtn->setObjectName("simImportBtn");
    btnLayout->addWidget(m_importBtn);

    mainLayout->addLayout(btnLayout);

    // 统计标签
    m_statsLabel = new QLabel(this);
    m_statsLabel->setObjectName("simStatsLabel");
    mainLayout->addWidget(m_statsLabel);

    // 信号连接
    connect(m_addBtn, &QPushButton::clicked, this, &DeviceSimulatorPanel::onAddRule);
    connect(m_removeBtn, &QPushButton::clicked, this, &DeviceSimulatorPanel::onRemoveRule);
    connect(m_clearBtn, &QPushButton::clicked, this, &DeviceSimulatorPanel::onClearRules);
    connect(m_exportBtn, &QPushButton::clicked, this, &DeviceSimulatorPanel::onExportClicked);
    connect(m_importBtn, &QPushButton::clicked, this, &DeviceSimulatorPanel::onImportClicked);
    connect(m_table, &QTableWidget::cellChanged, this, &DeviceSimulatorPanel::onCellChanged);
    connect(m_echoCheck, &QCheckBox::toggled, this, [this]() { syncToSimulator(); emit configChanged(); });
    connect(m_noiseSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this]() { syncToSimulator(); emit configChanged(); });
    connect(m_minDelaySpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() { syncToSimulator(); emit configChanged(); });
    connect(m_maxDelaySpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() { syncToSimulator(); emit configChanged(); });
}

void DeviceSimulatorPanel::loadFromSimulator()
{
    if (!m_simulator) return;
    m_syncing = true;
    m_table->setRowCount(0);
    const auto& rules = m_simulator->responses();
    for (int i = 0; i < rules.size(); ++i) {
        m_table->insertRow(i);
        fillRow(i, rules[i]);
    }
    m_echoCheck->setChecked(m_simulator->autoEcho());
    m_noiseSpin->setValue(m_simulator->noiseRate());
    m_syncing = false;
}

void DeviceSimulatorPanel::syncToSimulator()
{
    if (!m_simulator) return;
    m_simulator->clearResponses();
    for (int i = 0; i < m_table->rowCount(); ++i)
        m_simulator->addResponse(buildRuleFromRow(i));
    m_simulator->setAutoEcho(m_echoCheck->isChecked());
    m_simulator->setNoiseRate(m_noiseSpin->value());
    m_simulator->setDefaultDelay(m_minDelaySpin->value(), m_maxDelaySpin->value());
}

SimResponse DeviceSimulatorPanel::buildRuleFromRow(int row) const
{
    SimResponse r;
    auto* cmdItem = m_table->item(row, 0);
    if (cmdItem) r.commandPattern = QByteArray::fromHex(cmdItem->text().toUtf8());

    auto* stratCombo = qobject_cast<QComboBox*>(m_table->cellWidget(row, 1));
    if (stratCombo) r.strategy = static_cast<MatchStrategy>(stratCombo->currentIndex());

    auto* modeCombo = qobject_cast<QComboBox*>(m_table->cellWidget(row, 2));
    if (modeCombo) r.mode = static_cast<ResponseMode>(modeCombo->currentIndex());

    auto* dataItem = m_table->item(row, 3);
    if (dataItem) r.fixedData = QByteArray::fromHex(dataItem->text().toUtf8());

    auto* minItem = m_table->item(row, 4);
    if (minItem) r.minDelayMs = minItem->text().toInt();

    auto* maxItem = m_table->item(row, 5);
    if (maxItem) r.maxDelayMs = maxItem->text().toInt();

    auto* enableItem = m_table->item(row, 6);
    if (enableItem) {
        auto* cb = qobject_cast<QCheckBox*>(m_table->cellWidget(row, 6));
        r.enabled = cb ? cb->isChecked() : true;
    }
    return r;
}

void DeviceSimulatorPanel::fillRow(int row, const SimResponse& rule)
{
    // 列0: 命令模式(HEX)
    auto* cmdItem = new QTableWidgetItem(QString::fromUtf8(rule.commandPattern.toHex()));
    m_table->setItem(row, 0, cmdItem);

    // 列1: 匹配策略(ComboBox)
    auto* stratCombo = new QComboBox(this);
    stratCombo->setObjectName("simStratCombo_" + QString::number(row));
    stratCombo->addItems({tr("精确匹配"), tr("前缀匹配"), tr("正则匹配")});
    stratCombo->setCurrentIndex(static_cast<int>(rule.strategy));
    m_table->setCellWidget(row, 1, stratCombo);

    // 列2: 响应模式(ComboBox)
    auto* modeCombo = new QComboBox(this);
    modeCombo->setObjectName("simModeCombo_" + QString::number(row));
    modeCombo->addItems({tr("固定"), tr("递增"), tr("随机"), tr("脚本")});
    modeCombo->setCurrentIndex(static_cast<int>(rule.mode));
    m_table->setCellWidget(row, 2, modeCombo);

    // 列3: 响应数据(HEX)
    auto* dataItem = new QTableWidgetItem(QString::fromUtf8(rule.fixedData.toHex()));
    m_table->setItem(row, 3, dataItem);

    // 列4: 最小延迟
    auto* minItem = new QTableWidgetItem(QString::number(rule.minDelayMs));
    m_table->setItem(row, 4, minItem);

    // 列5: 最大延迟
    auto* maxItem = new QTableWidgetItem(QString::number(rule.maxDelayMs));
    m_table->setItem(row, 5, maxItem);

    // 列6: 启用(CheckBox)
    auto* enableCb = new QCheckBox(this);
    enableCb->setObjectName("simEnableCb_" + QString::number(row));
    enableCb->setChecked(rule.enabled);
    m_table->setCellWidget(row, 6, enableCb);
}

QString DeviceSimulatorPanel::modeToString(ResponseMode mode) const
{
    switch (mode) {
    case ResponseMode::Fixed:      return tr("固定");
    case ResponseMode::Incremental: return tr("递增");
    case ResponseMode::Random:     return tr("随机");
    case ResponseMode::Scripted:   return tr("脚本");
    }
    return tr("未知");
}

QString DeviceSimulatorPanel::strategyToString(MatchStrategy s) const
{
    switch (s) {
    case MatchStrategy::Exact:  return tr("精确匹配");
    case MatchStrategy::Prefix: return tr("前缀匹配");
    case MatchStrategy::Regex:  return tr("正则匹配");
    }
    return tr("未知");
}
