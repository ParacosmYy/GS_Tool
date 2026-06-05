/**
 * @file NotificationHistory.cpp
 * @brief 通知历史记录控件实现 -- 5级过滤/搜索/JSON+CSV导出
 */
#include "core/notification/NotificationHistory.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileDialog>
#include <QScrollBar>
#include <QApplication>

// ─── 构造/析构 ────────────────────────────────────────────────

/**
 * @brief 构造通知历史记录控件
 * @param maxHistory 最大历史条目数
 * @param parent 父控件
 */
NotificationHistory::NotificationHistory(int maxHistory, QWidget* parent)
    : QWidget(parent)
    , m_maxHistory(maxHistory)
{
    setObjectName("NotificationHistory");
    setupUI();
}

// ─── 公开API ──────────────────────────────────────────────────

/**
 * @brief 添加通知条目(参数版)
 * @param level 级别
 * @param title 标题
 * @param message 消息内容
 * @param source 来源组件标识
 */
void NotificationHistory::addEntry(Level level, const QString& title,
                                   const QString& message, const QString& source)
{
    NotificationEntry entry;
    entry.timestamp = QDateTime::currentMSecsSinceEpoch();
    entry.level = level;
    entry.title = title;
    entry.message = message;
    entry.source = source;
    addEntry(entry);
}

/**
 * @brief 添加通知条目(结构体版)
 * @param entry 完整通知条目
 */
void NotificationHistory::addEntry(const NotificationEntry& entry)
{
    NotificationEntry e = entry;
    e.id = ++m_nextId;

    // 新条目插入到最前面(最新在前)
    m_entries.prepend(e);

    // 裁剪超出最大条数的旧记录
    while (m_entries.size() > m_maxHistory) {
        m_entries.removeLast();
    }

    // 更新统计
    ++m_stats.totalNotifications;
    const int levelIdx = static_cast<int>(e.level);
    if (levelIdx >= 0 && levelIdx < 5) {
        ++m_stats.totalByLevel[levelIdx];
    }
    if (m_entries.size() > m_stats.peakHistorySize) {
        m_stats.peakHistorySize = m_entries.size();
    }

    // 刷新视图并自动滚动
    refreshView();

    emit entryAdded(e.id);
}

/**
 * @brief 获取过滤后的条目列表
 * @return 通过过滤条件的条目
 */
QList<NotificationHistory::NotificationEntry> NotificationHistory::filteredEntries() const
{
    QList<NotificationEntry> result;
    for (const auto& e : m_entries) {
        if (matchesFilter(e)) {
            result.append(e);
        }
    }
    return result;
}

/**
 * @brief 设置最低显示级别
 * @param minLevel 只显示>=此级别的通知
 */
void NotificationHistory::setFilterLevel(Level minLevel)
{
    if (m_filterLevel == minLevel) return;
    m_filterLevel = minLevel;
    refreshView();
    emit filterChanged(minLevel);
}

/**
 * @brief 设置搜索文本(匹配标题+消息+来源)
 * @param text 搜索关键字，空则不过滤
 */
void NotificationHistory::setSearchText(const QString& text)
{
    if (m_searchText == text) return;
    m_searchText = text;
    ++m_stats.totalSearches;
    refreshView();
}

/**
 * @brief 清除所有历史记录
 */
void NotificationHistory::clearHistory()
{
    ++m_stats.totalClears;
    m_entries.clear();
    refreshView();
    emit historyCleared();
}

/**
 * @brief 获取过滤后条目数
 * @return 过滤结果数量
 */
int NotificationHistory::filteredCount() const
{
    int count = 0;
    for (const auto& e : m_entries) {
        if (matchesFilter(e)) ++count;
    }
    return count;
}

/**
 * @brief 导出为JSON文件
 * @param filePath 目标文件路径
 * @return 是否成功
 */
bool NotificationHistory::exportToJson(const QString& filePath)
{
    QList<NotificationEntry> items = filteredEntries();
    QJsonArray arr;
    for (const auto& e : items) {
        QJsonObject obj;
        obj["id"] = e.id;
        obj["timestamp"] = QString::number(e.timestamp);
        obj["timeFormatted"] = formatTimestamp(e.timestamp);
        obj["level"] = levelToString(e.level);
        obj["title"] = e.title;
        obj["message"] = e.message;
        obj["source"] = e.source;
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    ++m_stats.totalExports;
    return true;
}

/**
 * @brief 导出为CSV文件
 * @param filePath 目标文件路径
 * @return 是否成功
 */
bool NotificationHistory::exportToCsv(const QString& filePath)
{
    QList<NotificationEntry> items = filteredEntries();
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    // CSV头
    file.write("ID,Time,Level,Source,Title,Message\n");

    for (const auto& e : items) {
        // 简易CSV转义: 双引号包裹，内部双引号翻倍
        auto escape = [](const QString& s) -> QByteArray {
            QString out = s;
            out.replace("\"", "\"\"");
            return ("\"" + out + "\"").toUtf8();
        };

        QByteArray line;
        line += QByteArray::number(e.id) + ",";
        line += formatTimestamp(e.timestamp).toUtf8() + ",";
        line += levelToString(e.level).toUtf8() + ",";
        line += escape(e.source) + ",";
        line += escape(e.title) + ",";
        line += escape(e.message) + "\n";
        file.write(line);
    }

    file.close();
    ++m_stats.totalExports;
    return true;
}

// ─── UI构建 ───────────────────────────────────────────────────

/**
 * @brief 构建UI布局 -- 工具栏(过滤+搜索+导出) + 表格 + 状态栏
 */
void NotificationHistory::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    // ---- 工具栏 ----
    auto* toolbar = new QHBoxLayout();
    toolbar->setSpacing(8);

    // 级别过滤
    auto* levelLabel = new QLabel(tr("Level:"), this);
    levelLabel->setObjectName("NotificationHistoryLevelLabel");
    toolbar->addWidget(levelLabel);

    m_levelCombo = new QComboBox(this);
    m_levelCombo->setObjectName("NotificationHistoryLevelCombo");
    m_levelCombo->addItem(tr("All"), static_cast<int>(Level::Debug));
    m_levelCombo->addItem(levelToString(Level::Debug), static_cast<int>(Level::Debug));
    m_levelCombo->addItem(levelToString(Level::Info), static_cast<int>(Level::Info));
    m_levelCombo->addItem(levelToString(Level::Warning), static_cast<int>(Level::Warning));
    m_levelCombo->addItem(levelToString(Level::Error), static_cast<int>(Level::Error));
    m_levelCombo->addItem(levelToString(Level::Critical), static_cast<int>(Level::Critical));
    // 默认选中"Debug"即显示全部(最低级别)
    connect(m_levelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        Level lvl = static_cast<Level>(m_levelCombo->currentData().toInt());
        setFilterLevel(lvl);
    });
    toolbar->addWidget(m_levelCombo);

    // 搜索框
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName("NotificationHistorySearchEdit");
    m_searchEdit->setPlaceholderText(tr("Search title/message/source..."));
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &NotificationHistory::setSearchText);
    toolbar->addWidget(m_searchEdit, 1);

    // 导出JSON按钮
    m_exportJsonBtn = new QPushButton(tr("Export JSON"), this);
    m_exportJsonBtn->setObjectName("NotificationHistoryExportJsonBtn");
    connect(m_exportJsonBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getSaveFileName(this, tr("Export JSON"),
                       QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + "_notifications.json",
                       tr("JSON Files (*.json)"));
        if (!path.isEmpty()) {
            if (exportToJson(path)) {
                m_statusLabel->setText(tr("Exported %1 entries to JSON").arg(filteredCount()));
            } else {
                m_statusLabel->setText(tr("JSON export failed"));
            }
        }
    });
    toolbar->addWidget(m_exportJsonBtn);

    // 导出CSV按钮
    m_exportCsvBtn = new QPushButton(tr("Export CSV"), this);
    m_exportCsvBtn->setObjectName("NotificationHistoryExportCsvBtn");
    connect(m_exportCsvBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getSaveFileName(this, tr("Export CSV"),
                       QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + "_notifications.csv",
                       tr("CSV Files (*.csv)"));
        if (!path.isEmpty()) {
            if (exportToCsv(path)) {
                m_statusLabel->setText(tr("Exported %1 entries to CSV").arg(filteredCount()));
            } else {
                m_statusLabel->setText(tr("CSV export failed"));
            }
        }
    });
    toolbar->addWidget(m_exportCsvBtn);

    // 清除按钮
    m_clearBtn = new QPushButton(tr("Clear"), this);
    m_clearBtn->setObjectName("NotificationHistoryClearBtn");
    connect(m_clearBtn, &QPushButton::clicked, this, &NotificationHistory::clearHistory);
    toolbar->addWidget(m_clearBtn);

    mainLayout->addLayout(toolbar);

    // ---- 表格 ----
    m_table = new QTableWidget(this);
    m_table->setObjectName("NotificationHistoryTable");
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({
        tr("Time"), tr("Level"), tr("Source"), tr("Title"), tr("Message")
    });
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(false);

    mainLayout->addWidget(m_table, 1);

    // ---- 状态栏 ----
    m_statusLabel = new QLabel(tr("No notifications"), this);
    m_statusLabel->setObjectName("NotificationHistoryStatusLabel");
    mainLayout->addWidget(m_statusLabel);
}

// ─── 视图刷新 ─────────────────────────────────────────────────

/**
 * @brief 刷新表格视图 -- 根据当前过滤器重绘所有行，自动滚动到最新
 */
void NotificationHistory::refreshView()
{
    // 保存滚动位置
    QScrollBar* vScroll = m_table->verticalScrollBar();
    bool atTop = (vScroll->value() <= vScroll->minimum() + 5);

    m_table->setRowCount(0);

    int shownCount = 0;
    for (const auto& e : m_entries) {
        if (!matchesFilter(e)) continue;

        int row = m_table->rowCount();
        m_table->insertRow(row);

        // 时间
        auto* timeItem = new QTableWidgetItem(formatTimestamp(e.timestamp));
        timeItem->setForeground(QColor(120, 120, 120));
        m_table->setItem(row, 0, timeItem);

        // 级别(颜色编码)
        auto* levelItem = new QTableWidgetItem(levelToString(e.level));
        levelItem->setForeground(levelColor(e.level));
        m_table->setItem(row, 1, levelItem);

        // 来源
        auto* sourceItem = new QTableWidgetItem(e.source);
        m_table->setItem(row, 2, sourceItem);

        // 标题
        auto* titleItem = new QTableWidgetItem(e.title);
        m_table->setItem(row, 3, titleItem);

        // 消息
        auto* msgItem = new QTableWidgetItem(e.message);
        m_table->setItem(row, 4, msgItem);

        // 整行颜色标记(最左侧级别色条视觉提示)
        QColor rowBg = levelColor(e.level);
        rowBg.setAlpha(20);
        for (int c = 0; c < 5; ++c) {
            if (m_table->item(row, c)) {
                m_table->item(row, c)->setBackground(rowBg);
            }
        }

        ++shownCount;
    }

    // 自动滚动到最新(表格顶部) -- 除非用户已手动向下滚动浏览历史
    if (atTop && m_table->rowCount() > 0) {
        m_table->scrollToTop();
    }

    // 更新状态栏
    if (m_entries.isEmpty()) {
        m_statusLabel->setText(tr("No notifications"));
    } else if (shownCount < m_entries.size()) {
        m_statusLabel->setText(tr("Showing %1 of %2 notifications")
                               .arg(shownCount).arg(m_entries.size()));
    } else {
        m_statusLabel->setText(tr("Total: %1 notifications").arg(m_entries.size()));
    }
}

// ─── 私有工具方法 ─────────────────────────────────────────────

/**
 * @brief 级别枚举转可读字符串
 * @param level 通知级别
 * @return 中文级别名称
 */
QString NotificationHistory::levelToString(Level level) const
{
    switch (level) {
    case Level::Debug:    return tr("Debug");
    case Level::Info:     return tr("Info");
    case Level::Warning:  return tr("Warning");
    case Level::Error:    return tr("Error");
    case Level::Critical: return tr("Critical");
    }
    return tr("Unknown");
}

/**
 * @brief 获取级别对应的语义颜色
 * @param level 通知级别
 * @return QColor语义色
 */
QColor NotificationHistory::levelColor(Level level) const
{
    switch (level) {
    case Level::Debug:    return QColor(120, 120, 120);  ///< 灰色
    case Level::Info:     return QColor(66, 133, 244);   ///< 蓝色
    case Level::Warning:  return QColor(234, 179, 8);    ///< 黄色
    case Level::Error:    return QColor(239, 68, 68);    ///< 红色
    case Level::Critical: return QColor(220, 38, 38);    ///< 深红色
    }
    return QColor(120, 120, 120);
}

/**
 * @brief 格式化时间戳为可读字符串
 * @param ts 毫秒时间戳
 * @return HH:mm:ss.zzz 格式
 */
QString NotificationHistory::formatTimestamp(qint64 ts) const
{
    return QDateTime::fromMSecsSinceEpoch(ts).toString("HH:mm:ss.zzz");
}

/**
 * @brief 判断条目是否通过当前过滤条件
 * @param entry 待判断条目
 * @return true=通过(应显示)
 */
bool NotificationHistory::matchesFilter(const NotificationEntry& entry) const
{
    // 级别过滤
    if (static_cast<int>(entry.level) < static_cast<int>(m_filterLevel)) {
        return false;
    }

    // 文本搜索(空则不过滤)
    if (!m_searchText.isEmpty()) {
        QString keyword = m_searchText.toLower();
        if (!entry.title.toLower().contains(keyword) &&
            !entry.message.toLower().contains(keyword) &&
            !entry.source.toLower().contains(keyword)) {
            return false;
        }
    }

    return true;
}
