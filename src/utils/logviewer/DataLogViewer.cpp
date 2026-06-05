/**
 * @file DataLogViewer.cpp
 * @brief 数据日志查看器实现 — 加载/搜索/过滤/导出
 */

#include "utils/logviewer/DataLogViewer.h"

#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QDateTimeEdit>
#include <QCheckBox>
#include <QTableWidget>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDateTime>
#include <QApplication>
#include <QClipboard>

/** @brief 构造函数 @param parent 父控件 */
DataLogViewer::DataLogViewer(QWidget* parent)
    : QWidget(parent)
    , m_directionFilter(-1)
    , m_timeFrom(-1)
    , m_timeTo(-1)
{
    setupUI();
}

/** @brief 初始化UI布局 */
void DataLogViewer::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    /* 工具栏: 搜索+过滤 */
    auto* toolbar = new QHBoxLayout();

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName(QStringLiteral("logSearchEdit"));
    m_searchEdit->setPlaceholderText(tr("搜索关键字..."));

    m_regexCheck = new QCheckBox(tr("正则"), this);
    m_regexCheck->setObjectName(QStringLiteral("logRegexCheck"));

    m_directionCombo = new QComboBox(this);
    m_directionCombo->setObjectName(QStringLiteral("logDirectionCombo"));
    m_directionCombo->addItem(tr("全部"), -1);
    m_directionCombo->addItem(tr("接收(RX)"), 0);
    m_directionCombo->addItem(tr("发送(TX)"), 1);

    m_timeFilterCheck = new QCheckBox(tr("时间范围"), this);
    m_timeFilterCheck->setObjectName(QStringLiteral("logTimeCheck"));

    m_fromTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-1), this);
    m_fromTimeEdit->setObjectName(QStringLiteral("logFromTime"));
    m_fromTimeEdit->setCalendarPopup(true);
    m_fromTimeEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    m_fromTimeEdit->setEnabled(false);

    m_toTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    m_toTimeEdit->setObjectName(QStringLiteral("logToTime"));
    m_toTimeEdit->setCalendarPopup(true);
    m_toTimeEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    m_toTimeEdit->setEnabled(false);

    m_searchBtn = new QPushButton(tr("搜索"), this);
    m_searchBtn->setObjectName(QStringLiteral("logSearchBtn"));

    m_loadBtn = new QPushButton(tr("加载"), this);
    m_loadBtn->setObjectName(QStringLiteral("logLoadBtn"));

    m_exportBtn = new QPushButton(tr("导出"), this);
    m_exportBtn->setObjectName(QStringLiteral("logExportBtn"));

    toolbar->addWidget(m_searchEdit, 3);
    toolbar->addWidget(m_regexCheck);
    toolbar->addWidget(m_directionCombo);
    toolbar->addWidget(m_timeFilterCheck);
    toolbar->addWidget(m_fromTimeEdit);
    toolbar->addWidget(m_toTimeEdit);
    toolbar->addWidget(m_searchBtn);
    toolbar->addWidget(m_loadBtn);
    toolbar->addWidget(m_exportBtn);

    mainLayout->addLayout(toolbar);

    /* 日志表格 */
    m_logTable = new QTableWidget(0, 5, this);
    m_logTable->setObjectName(QStringLiteral("logTable"));
    m_logTable->setHorizontalHeaderLabels({
        tr("时间"), tr("方向"), tr("Hex"), tr("ASCII"), tr("长度")
    });
    m_logTable->horizontalHeader()->setStretchLastSection(true);
    m_logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_logTable->setAlternatingRowColors(true);
    mainLayout->addWidget(m_logTable, 1);

    /* 状态栏 */
    auto* statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel(tr("就绪"), this);
    m_statusLabel->setObjectName(QStringLiteral("logStatusLabel"));

    m_progressBar = new QProgressBar(this);
    m_progressBar->setObjectName(QStringLiteral("logProgress"));
    m_progressBar->setMaximum(100);
    m_progressBar->setVisible(false);

    statusLayout->addWidget(m_statusLabel, 1);
    statusLayout->addWidget(m_progressBar);
    mainLayout->addLayout(statusLayout);

    /* 信号连接 */
    connect(m_searchBtn, &QPushButton::clicked,
            this, &DataLogViewer::onSearchClicked);
    connect(m_loadBtn, &QPushButton::clicked,
            this, &DataLogViewer::onLoadClicked);
    connect(m_exportBtn, &QPushButton::clicked,
            this, &DataLogViewer::onExportClicked);
    connect(m_directionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DataLogViewer::onFilterChanged);
    connect(m_timeFilterCheck, &QCheckBox::toggled,
            m_fromTimeEdit, &QDateTimeEdit::setEnabled);
    connect(m_timeFilterCheck, &QCheckBox::toggled,
            m_toTimeEdit, &QDateTimeEdit::setEnabled);
    connect(m_timeFilterCheck, &QCheckBox::toggled,
            this, &DataLogViewer::onFilterChanged);
}

/** @brief 从文件加载日志 @param filePath 文件路径 @param format 格式 @return 是否成功 */
bool DataLogViewer::loadFromFile(const QString& filePath, LogFormat format)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_statusLabel->setText(tr("无法打开文件: %1").arg(filePath));
        return false;
    }

    QByteArray content = file.readAll();
    file.close();

    QList<DataLogEntry> entries;
    switch (format) {
    case LogFormat::Csv:
        entries = parseCsv(content);
        break;
    case LogFormat::HexDump:
        entries = parseHexDump(content);
        break;
    case LogFormat::RawBinary:
        /* 将整个文件作为单条记录 */
        {
            DataLogEntry entry;
            entry.timestamp = QDateTime::currentMSecsSinceEpoch();
            entry.data = content;
            entry.hexString = content.toHex(' ');
            entry.asciiString = QString::fromUtf8(content.left(256));
            entries.append(entry);
        }
        break;
    }

    loadEntries(entries);
    return true;
}

/** @brief 从内存加载条目 @param entries 日志条目列表 */
void DataLogViewer::loadEntries(const QList<DataLogEntry>& entries)
{
    m_allEntries = entries;
    m_stats.totalEntries += static_cast<quint64>(entries.size());
    if (entries.size() > m_stats.peakEntries) {
        m_stats.peakEntries = entries.size();
    }
    for (const auto& e : entries) {
        m_stats.totalBytesLoaded += static_cast<quint64>(e.data.size());
    }
    applyFilters();
    emit dataLoaded(entries.size());
    m_statusLabel->setText(tr("已加载 %1 条记录").arg(entries.size()));
}

/** @brief 搜索关键字 @param keyword 搜索词 @param useRegex 是否正则 */
void DataLogViewer::search(const QString& keyword, bool useRegex)
{
    m_currentKeyword = keyword;
    ++m_stats.totalSearches;
    applyFilters();
}

/** @brief 设置方向过滤器 @param direction 方向 */
void DataLogViewer::setDirectionFilter(int direction)
{
    m_directionFilter = direction;
    ++m_stats.totalFilters;
    applyFilters();
}

/** @brief 设置时间范围 @param fromMs 起始时间 @param toMs 结束时间 */
void DataLogViewer::setTimeFilter(qint64 fromMs, qint64 toMs)
{
    m_timeFrom = fromMs;
    m_timeTo = toMs;
    ++m_stats.totalFilters;
    applyFilters();
}

/** @brief 应用所有过滤条件到日志列表 */
void DataLogViewer::applyFilters()
{
    m_filteredEntries.clear();
    int matched = 0;

    /* 构建正则表达式(如果需要) */
    QRegularExpression regex;
    if (!m_currentKeyword.isEmpty() && m_regexCheck->isChecked()) {
        regex.setPattern(m_currentKeyword);
    }

    for (const auto& entry : m_allEntries) {
        /* 方向过滤 */
        if (m_directionFilter >= 0 && entry.direction != m_directionFilter) {
            continue;
        }

        /* 时间范围过滤 */
        if (m_timeFilterCheck->isChecked()) {
            qint64 fromMs = m_fromTimeEdit->dateTime().toMSecsSinceEpoch();
            qint64 toMs = m_toTimeEdit->dateTime().toMSecsSinceEpoch();
            if (entry.timestamp < fromMs || entry.timestamp > toMs) {
                continue;
            }
        }

        /* 关键字过滤 */
        if (!m_currentKeyword.isEmpty()) {
            bool found = false;
            if (m_regexCheck->isChecked() && regex.isValid()) {
                found = entry.hexString.contains(regex)
                     || entry.asciiString.contains(regex);
            } else {
                found = entry.hexString.contains(m_currentKeyword, Qt::CaseInsensitive)
                     || entry.asciiString.contains(m_currentKeyword, Qt::CaseInsensitive);
            }
            if (!found) continue;
        }

        ++matched;
        m_filteredEntries.append(entry);
    }

    m_stats.matchedResults += static_cast<quint64>(matched);
    refreshTable();
    emit searchCompleted(matched, m_allEntries.size());
    m_statusLabel->setText(tr("显示 %1/%2 条记录").arg(matched).arg(m_allEntries.size()));
}

/** @brief 刷新表格显示 */
void DataLogViewer::refreshTable()
{
    m_logTable->setRowCount(0);
    m_logTable->setUpdatesEnabled(false);

    for (int i = 0; i < m_filteredEntries.size(); ++i) {
        const auto& entry = m_filteredEntries.at(i);
        int row = m_logTable->rowCount();
        m_logTable->insertRow(row);

        QDateTime dt = QDateTime::fromMSecsSinceEpoch(entry.timestamp);
        m_logTable->setItem(row, 0, new QTableWidgetItem(dt.toString(QStringLiteral("HH:mm:ss.zzz"))));
        m_logTable->setItem(row, 1, new QTableWidgetItem(
            entry.direction == 0 ? tr("RX") : tr("TX")));
        m_logTable->setItem(row, 2, new QTableWidgetItem(entry.hexString.left(64)));
        m_logTable->setItem(row, 3, new QTableWidgetItem(entry.asciiString.left(64)));
        m_logTable->setItem(row, 4, new QTableWidgetItem(
            QString::number(entry.data.size())));
    }

    m_logTable->setUpdatesEnabled(true);
}

/** @brief 解析CSV格式日志 @param content 文件内容 @return 日志条目列表 */
QList<DataLogEntry> DataLogViewer::parseCsv(const QByteArray& content)
{
    QList<DataLogEntry> entries;
    QTextStream stream(content);
    QString line;

    while (stream.readLineInto(&line)) {
        if (line.startsWith(QLatin1Char('#')) || line.trimmed().isEmpty()) {
            continue;
        }
        QStringList parts = line.split(QLatin1Char(','));
        if (parts.size() < 3) continue;

        DataLogEntry entry;
        entry.timestamp = parts[0].toLongLong();
        entry.direction = parts[1].toInt();
        entry.hexString = parts[2].trimmed();
        entry.data = QByteArray::fromHex(entry.hexString.toUtf8());
        if (parts.size() > 3) {
            entry.asciiString = parts[3].trimmed();
        } else {
            entry.asciiString = QString::fromUtf8(entry.data);
        }
        entries.append(entry);
    }
    return entries;
}

/** @brief 解析HEX转储格式 @param content 文件内容 @return 日志条目列表 */
QList<DataLogEntry> DataLogViewer::parseHexDump(const QByteArray& content)
{
    QList<DataLogEntry> entries;
    QTextStream stream(content);
    QString line;
    qint64 baseTime = QDateTime::currentMSecsSinceEpoch();

    while (stream.readLineInto(&line)) {
        /* 标准HEX转储: "00000000: 48 65 6C 6C 6F" */
        int colonPos = line.indexOf(QLatin1Char(':'));
        if (colonPos < 0) continue;

        QString hexPart = line.mid(colonPos + 1).trimmed();
        QStringList hexBytes = hexPart.split(QLatin1Char(' '), Qt::SkipEmptyParts);

        QByteArray data;
        for (const QString& h : hexBytes) {
            bool ok = false;
            char byte = static_cast<char>(h.toUInt(&ok, 16));
            if (ok) data.append(byte);
        }

        if (!data.isEmpty()) {
            DataLogEntry entry;
            entry.timestamp = baseTime + entries.size();
            entry.data = data;
            entry.hexString = data.toHex(' ').toUpper();
            entry.asciiString = QString::fromUtf8(data);
            entries.append(entry);
        }
    }
    return entries;
}

/** @brief 导出过滤后的结果 @param filePath 目标路径 @return 是否成功 */
bool DataLogViewer::exportFiltered(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << tr("# 时间,方向,Hex,ASCII,长度\n");

    for (const auto& entry : m_filteredEntries) {
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(entry.timestamp);
        out << dt.toString(Qt::ISODateWithMs) << QLatin1Char(',')
            << (entry.direction == 0 ? QStringLiteral("RX") : QStringLiteral("TX"))
            << QLatin1Char(',') << entry.hexString << QLatin1Char(',')
            << entry.asciiString << QLatin1Char(',')
            << entry.data.size() << QLatin1Char('\n');
    }

    file.close();
    ++m_stats.totalExports;
    qint64 bytes = file.size();
    emit exportCompleted(filePath, bytes);
    return true;
}

/** @brief 清除所有数据 */
void DataLogViewer::clear()
{
    m_allEntries.clear();
    m_filteredEntries.clear();
    m_currentKeyword.clear();
    m_logTable->setRowCount(0);
    m_statusLabel->setText(tr("就绪"));
}

/** @brief 搜索按钮点击 */
void DataLogViewer::onSearchClicked()
{
    search(m_searchEdit->text(), m_regexCheck->isChecked());
}

/** @brief 过滤条件变化 */
void DataLogViewer::onFilterChanged()
{
    int dir = m_directionCombo->currentData().toInt();
    m_directionFilter = dir;
    applyFilters();
}

/** @brief 导出按钮点击 */
void DataLogViewer::onExportClicked()
{
    QString path = QFileDialog::getSaveFileName(this,
        tr("导出日志"), QString(), tr("CSV文件 (*.csv)"));
    if (!path.isEmpty()) {
        if (exportFiltered(path)) {
            m_statusLabel->setText(tr("已导出到: %1").arg(path));
        }
    }
}

/** @brief 加载按钮点击 */
void DataLogViewer::onLoadClicked()
{
    QString path = QFileDialog::getOpenFileName(this,
        tr("加载日志"), QString(),
        tr("CSV文件 (*.csv);;HEX文件 (*.hex);;所有文件 (*)"));
    if (path.isEmpty()) return;

    LogFormat fmt = path.endsWith(QLatin1String(".csv")) ? LogFormat::Csv : LogFormat::HexDump;
    loadFromFile(path, fmt);
}
