/**
 * @file DataCompareWidget.cpp
 * @brief 数据字节级对比控件实现
 * @author Serial Tool Team
 * @date 2026-06-05
 *
 * 包含构造函数、setupUI、setCompareData、compare、
 * updateResultTable、updateSummary、导航和槽函数。
 */

#include "utils/compare/DataCompareWidget.h"

#include <QApplication>
#include <QBrush>
#include <QClipboard>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringList>

/** @brief 构造函数：初始化界面 @param parent 父控件 */
DataCompareWidget::DataCompareWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentDiffIndex(-1)
{
    setupUI();
}

/** @brief 初始化界面：QSplitter 左右布局，左侧输入区，右侧结果区 */
void DataCompareWidget::setupUI()
{
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName(QStringLiteral("compareSplitter"));

    /* ── 左侧: 两个 Hex 输入区 + 对比按钮 ── */
    auto *leftWidget = new QWidget(splitter);
    leftWidget->setObjectName(QStringLiteral("compareInputPanel"));
    auto *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(2, 2, 2, 2);

    /* 数据A输入 */
    auto *labelA = new QLabel(tr("数据 A (Hex):"), leftWidget);
    labelA->setObjectName(QStringLiteral("compareLabelA"));
    leftLayout->addWidget(labelA);

    m_dataAEdit = new QTextEdit(leftWidget);
    m_dataAEdit->setObjectName(QStringLiteral("compareDataAEdit"));
    m_dataAEdit->setPlaceholderText(tr("在此输入十六进制数据，如: 48 65 6C 6C 6F"));
    m_dataAEdit->setMaximumHeight(120);
    leftLayout->addWidget(m_dataAEdit);

    /* 数据B输入 */
    auto *labelB = new QLabel(tr("数据 B (Hex):"), leftWidget);
    labelB->setObjectName(QStringLiteral("compareLabelB"));
    leftLayout->addWidget(labelB);

    m_dataBEdit = new QTextEdit(leftWidget);
    m_dataBEdit->setObjectName(QStringLiteral("compareDataBEdit"));
    m_dataBEdit->setPlaceholderText(tr("在此输入十六进制数据，如: 48 65 6C 6C 6F"));
    m_dataBEdit->setMaximumHeight(120);
    leftLayout->addWidget(m_dataBEdit);

    /* 对比按钮 */
    m_compareBtn = new QPushButton(tr("开始对比"), leftWidget);
    m_compareBtn->setObjectName(QStringLiteral("compareStartBtn"));
    leftLayout->addWidget(m_compareBtn);
    leftLayout->addStretch();

    /* ── 右侧: 结果表格 + 导航 + 摘要 ── */
    auto *rightWidget = new QWidget(splitter);
    rightWidget->setObjectName(QStringLiteral("compareResultPanel"));
    auto *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(2, 2, 2, 2);

    /* 结果表格 */
    m_resultTable = new QTableWidget(rightWidget);
    m_resultTable->setObjectName(QStringLiteral("compareResultTable"));
    m_resultTable->setColumnCount(4);
    m_resultTable->setHorizontalHeaderLabels(
        QStringList() << tr("偏移") << tr("Hex A") << tr("Hex B") << tr("类型"));
    m_resultTable->horizontalHeader()->setStretchLastSection(true);
    m_resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    rightLayout->addWidget(m_resultTable);

    /* 导航按钮行 */
    auto *navLayout = new QHBoxLayout();
    navLayout->setObjectName(QStringLiteral("compareNavLayout"));

    m_prevDiffBtn = new QPushButton(tr("上一个差异"), rightWidget);
    m_prevDiffBtn->setObjectName(QStringLiteral("comparePrevDiffBtn"));
    navLayout->addWidget(m_prevDiffBtn);

    m_nextDiffBtn = new QPushButton(tr("下一个差异"), rightWidget);
    m_nextDiffBtn->setObjectName(QStringLiteral("compareNextDiffBtn"));
    navLayout->addWidget(m_nextDiffBtn);

    m_copyBtn = new QPushButton(tr("复制结果"), rightWidget);
    m_copyBtn->setObjectName(QStringLiteral("compareCopyBtn"));
    navLayout->addWidget(m_copyBtn);

    navLayout->addStretch();
    rightLayout->addLayout(navLayout);

    /* 摘要标签 */
    m_summaryLabel = new QLabel(tr("尚未执行对比"), rightWidget);
    m_summaryLabel->setObjectName(QStringLiteral("compareSummaryLabel"));
    rightLayout->addWidget(m_summaryLabel);

    /* 组装 Splitter */
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setSizes(QList<int>() << 300 << 500);
    mainLayout->addWidget(splitter);

    /* 信号连接 */
    connect(m_compareBtn, &QPushButton::clicked,
            this, &DataCompareWidget::onCompareClicked);
    connect(m_nextDiffBtn, &QPushButton::clicked,
            this, &DataCompareWidget::nextDiff);
    connect(m_prevDiffBtn, &QPushButton::clicked,
            this, &DataCompareWidget::prevDiff);
    connect(m_copyBtn, &QPushButton::clicked,
            this, &DataCompareWidget::onCopyResult);
}

/** @brief 设置待对比的数据并同步到文本编辑器 @param dataA 数据A @param dataB 数据B */
void DataCompareWidget::setCompareData(const QByteArray &dataA, const QByteArray &dataB)
{
    m_dataA = dataA;
    m_dataB = dataB;
    m_dataAEdit->setPlainText(QString::fromLatin1(dataA.toHex(' ')).toUpper());
    m_dataBEdit->setPlainText(QString::fromLatin1(dataB.toHex(' ')).toUpper());
}

/**
 * @brief 执行字节级逐字节对比，构建 DiffEntry 列表并计算相似度
 * @return 对比结果（含差异列表和统计摘要）
 */
DataCompareWidget::CompareResult DataCompareWidget::compare()
{
    CompareResult result;
    result.totalBytes = qMax(m_dataA.size(), m_dataB.size());

    if (result.totalBytes == 0) {
        m_result = result;
        updateResultTable();
        updateSummary();
        emit compareCompleted(result);
        return result;
    }

    for (int i = 0; i < result.totalBytes; ++i) {
        DiffEntry entry;
        entry.offset = i;

        bool aExists = (i < m_dataA.size());
        bool bExists = (i < m_dataB.size());

        if (aExists && bExists) {
            entry.byteA = static_cast<quint8>(m_dataA[i]);
            entry.byteB = static_cast<quint8>(m_dataB[i]);
            if (entry.byteA == entry.byteB) {
                entry.type = Match;
                ++result.matchingBytes;
            } else {
                entry.type = Different;
                ++result.differentBytes;
                result.diffs.append(entry);
            }
        } else if (aExists) {
            entry.byteA = static_cast<quint8>(m_dataA[i]);
            entry.byteB = 0;
            entry.type = OnlyInA;
            ++result.onlyInA;
            result.diffs.append(entry);
        } else {
            entry.byteA = 0;
            entry.byteB = static_cast<quint8>(m_dataB[i]);
            entry.type = OnlyInB;
            ++result.onlyInB;
            result.diffs.append(entry);
        }
    }

    result.similarity = (result.totalBytes > 0)
        ? result.matchingBytes * 100.0 / result.totalBytes : 0.0;

    m_result = result;
    m_currentDiffIndex = -1;

    /* 更新累计统计 */
    ++m_stats.totalCompares;
    m_stats.totalBytesCompared += static_cast<quint64>(result.totalBytes);
    m_stats.totalDiffsFound += static_cast<quint64>(result.diffs.size());
    m_stats.peakSimilarity = qMax(m_stats.peakSimilarity, result.similarity);
    m_stats.lowestSimilarity = qMin(m_stats.lowestSimilarity, result.similarity);

    updateResultTable();
    updateSummary();
    emit compareCompleted(result);
    return result;
}

/** @brief 根据对比结果填充结果表格，根据差异类型设置行背景色 */
void DataCompareWidget::updateResultTable()
{
    m_resultTable->setRowCount(m_result.totalBytes);
    m_resultTable->setUpdatesEnabled(false);

    /* 颜色映射 */
    QBrush defaultBrush(Qt::white);
    QBrush diffBrush(QColor(255, 200, 200));    ///< 浅红 - 不同
    QBrush onlyABrush(QColor(255, 255, 200));   ///< 浅黄 - 仅在A
    QBrush onlyBBrush(QColor(200, 255, 255));   ///< 浅青 - 仅在B

    int idx = 0;
    for (int i = 0; i < m_result.totalBytes; ++i) {
        bool aExists = (i < m_dataA.size());
        bool bExists = (i < m_dataB.size());
        quint8 byteA = aExists ? static_cast<quint8>(m_dataA[i]) : 0;
        quint8 byteB = bExists ? static_cast<quint8>(m_dataB[i]) : 0;
        DiffType type = Match;

        /* 从 diffs 列表中查找当前偏移的类型 */
        if (idx < m_result.diffs.size() && m_result.diffs[idx].offset == i) {
            type = m_result.diffs[idx].type;
            ++idx;
        }

        /* 根据类型选择背景色 */
        QBrush bg = defaultBrush;
        if (type == Different) bg = diffBrush;
        else if (type == OnlyInA) bg = onlyABrush;
        else if (type == OnlyInB) bg = onlyBBrush;

        /* 偏移 */
        auto *itemOffset = new QTableWidgetItem(
            QStringLiteral("%1").arg(i, 4, 16, QChar('0')).toUpper());
        itemOffset->setBackground(bg);
        m_resultTable->setItem(i, 0, itemOffset);

        /* Hex A */
        auto *itemA = new QTableWidgetItem(
            aExists ? QStringLiteral("%1").arg(byteA, 2, 16, QChar('0')).toUpper()
                    : QStringLiteral("--"));
        itemA->setBackground(bg);
        m_resultTable->setItem(i, 1, itemA);

        /* Hex B */
        auto *itemB = new QTableWidgetItem(
            bExists ? QStringLiteral("%1").arg(byteB, 2, 16, QChar('0')).toUpper()
                    : QStringLiteral("--"));
        itemB->setBackground(bg);
        m_resultTable->setItem(i, 2, itemB);

        /* 类型 */
        QString typeStr;
        switch (type) {
        case Match:     typeStr = tr("匹配"); break;
        case Different: typeStr = tr("不同"); break;
        case OnlyInA:   typeStr = tr("仅在A"); break;
        case OnlyInB:   typeStr = tr("仅在B"); break;
        }
        auto *itemType = new QTableWidgetItem(typeStr);
        itemType->setBackground(bg);
        m_resultTable->setItem(i, 3, itemType);
    }

    m_resultTable->setUpdatesEnabled(true);
}

/** @brief 更新底部摘要标签文本 */
void DataCompareWidget::updateSummary()
{
    m_summaryLabel->setText(
        tr("匹配: %1字节, 差异: %2字节, 相似度: %3%")
            .arg(m_result.matchingBytes)
            .arg(m_result.differentBytes + m_result.onlyInA + m_result.onlyInB)
            .arg(m_result.similarity, 0, 'f', 1));
}

/**
 * @brief 导航到下一个差异位置，选中并滚动到对应行
 * @return 差异偏移量，无差异时返回 -1
 */
int DataCompareWidget::nextDiff()
{
    if (m_result.diffs.isEmpty()) {
        return -1;
    }

    ++m_stats.totalNavigations;
    m_currentDiffIndex = (m_currentDiffIndex + 1) % m_result.diffs.size();
    int offset = m_result.diffs[m_currentDiffIndex].offset;

    m_resultTable->selectRow(offset);
    m_resultTable->scrollToItem(m_resultTable->item(offset, 0));
    emit diffNavigated(offset);
    return offset;
}

/**
 * @brief 导航到上一个差异位置，选中并滚动到对应行
 * @return 差异偏移量，无差异时返回 -1
 */
int DataCompareWidget::prevDiff()
{
    if (m_result.diffs.isEmpty()) {
        return -1;
    }

    ++m_stats.totalNavigations;
    m_currentDiffIndex = (m_currentDiffIndex - 1 + m_result.diffs.size())
                         % m_result.diffs.size();
    int offset = m_result.diffs[m_currentDiffIndex].offset;

    m_resultTable->selectRow(offset);
    m_resultTable->scrollToItem(m_resultTable->item(offset, 0));
    emit diffNavigated(offset);
    return offset;
}

/** @brief "开始对比"按钮槽函数：解析 Hex 文本为 QByteArray 后执行对比 */
void DataCompareWidget::onCompareClicked()
{
    /* 将用户输入的 Hex 文本转换为 QByteArray */
    QString hexA = m_dataAEdit->toPlainText().simplified();
    QString hexB = m_dataBEdit->toPlainText().simplified();
    hexA.remove(QLatin1Char(' '));
    hexB.remove(QLatin1Char(' '));

    m_dataA = QByteArray::fromHex(hexA.toLatin1());
    m_dataB = QByteArray::fromHex(hexB.toLatin1());
    compare();
}

/** @brief "复制结果"按钮槽函数：将对比结果格式化为文本并复制到系统剪贴板 */
void DataCompareWidget::onCopyResult()
{
    ++m_stats.totalCopies;

    QString text = tr("=== 数据对比结果 ===\n");
    text += tr("总字节: %1\n").arg(m_result.totalBytes);
    text += tr("匹配: %1\n").arg(m_result.matchingBytes);
    text += tr("不同: %1\n").arg(m_result.differentBytes);
    text += tr("仅在A: %1\n").arg(m_result.onlyInA);
    text += tr("仅在B: %1\n").arg(m_result.onlyInB);
    text += tr("相似度: %1%\n").arg(m_result.similarity, 0, 'f', 1);
    text += tr("\n--- 差异明细 ---\n");

    for (const auto &d : m_result.diffs) {
        text += tr("偏移 0x%1  A=%2  B=%3  %4\n")
                    .arg(d.offset, 4, 16, QChar('0'))
                    .arg(d.byteA, 2, 16, QChar('0'))
                    .arg(d.byteB, 2, 16, QChar('0'))
                    .arg(d.type == Different ? tr("不同")
                        : d.type == OnlyInA ? tr("仅在A") : tr("仅在B"));
    }

    QApplication::clipboard()->setText(text);
}

/** @brief 获取最近一次对比结果 @return 对比结果 */
DataCompareWidget::CompareResult DataCompareWidget::result() const
{
    return m_result;
}
