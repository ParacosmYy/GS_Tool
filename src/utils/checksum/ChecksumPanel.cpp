/**
 * @file ChecksumPanel.cpp
 * @brief 校验和计算面板 UI 实现
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 支持十六进制/ASCII/文件三种输入模式，提供结果复制功能。
 */

#include "utils/checksum/ChecksumPanel.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QFile>
#include <QHBoxLayout>
#include <QVBoxLayout>

/**
 * @brief 构造函数，初始化面板布局
 */
ChecksumPanel::ChecksumPanel(QWidget *parent)
    : QWidget(parent)
    , m_inputEdit(new QTextEdit(this))
    , m_algoCombo(new QComboBox(this))
    , m_inputModeCombo(new QComboBox(this))
    , m_resultLabel(new QLabel(tr("结果：-"), this))
    , m_calcBtn(new QPushButton(tr("计算"), this))
    , m_copyBtn(new QPushButton(tr("复制结果"), this))
{
    setObjectName(QStringLiteral("ChecksumPanel"));

    auto *mainLayout = new QVBoxLayout(this);

    // 算法选择行
    auto *algoLayout = new QHBoxLayout();
    algoLayout->addWidget(new QLabel(tr("算法："), this));

    const auto algorithms = {
        ChecksumCalculator::CRC8, ChecksumCalculator::CRC16Ccitt,
        ChecksumCalculator::CRC16Modbus, ChecksumCalculator::CRC16Kermit,
        ChecksumCalculator::CRC32, ChecksumCalculator::CRC32C,
        ChecksumCalculator::Xor8, ChecksumCalculator::Sum8,
        ChecksumCalculator::Sum16, ChecksumCalculator::Sum32,
        ChecksumCalculator::CustomCrc
    };
    for (auto alg : algorithms) {
        m_algoCombo->addItem(ChecksumCalculator::algorithmName(alg),
                             static_cast<int>(alg));
    }
    algoLayout->addWidget(m_algoCombo);
    algoLayout->addStretch();
    algoLayout->addWidget(m_calcBtn);

    // 输入模式行
    auto *modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel(tr("输入模式："), this));

    m_inputModeCombo->setObjectName("inputModeCombo");
    m_inputModeCombo->addItem(tr("十六进制"), 0);
    m_inputModeCombo->addItem(tr("ASCII文本"), 1);
    m_inputModeCombo->addItem(tr("二进制文件"), 2);
    modeLayout->addWidget(m_inputModeCombo);
    modeLayout->addStretch();

    // 输入区
    m_inputEdit->setPlaceholderText(
        tr("输入十六进制数据（如：01 02 FF）..."));
    m_inputEdit->setMaximumHeight(100);

    // 结果行
    auto *resultLayout = new QHBoxLayout();
    m_resultLabel->setStyleSheet(
        QStringLiteral("font-weight: bold; font-size: 14pt;"));
    resultLayout->addWidget(m_resultLabel);
    resultLayout->addStretch();
    m_copyBtn->setObjectName("copyResultBtn");
    m_copyBtn->setEnabled(false);
    resultLayout->addWidget(m_copyBtn);

    mainLayout->addLayout(algoLayout);
    mainLayout->addLayout(modeLayout);
    mainLayout->addWidget(m_inputEdit);
    mainLayout->addLayout(resultLayout);

    // 连接信号
    connect(m_calcBtn, &QPushButton::clicked,
            this, &ChecksumPanel::onCalculate);
    connect(m_copyBtn, &QPushButton::clicked,
            this, &ChecksumPanel::onCopyResult);
}

/**
 * @brief 根据输入模式解析输入数据为字节数组
 */
QByteArray ChecksumPanel::inputData() const
{
    int mode = m_inputModeCombo->currentData().toInt();

    switch (mode) {
    case 0: {
        // 十六进制模式
        QString text = m_inputEdit->toPlainText().simplified();
        text.remove(' ');
        return QByteArray::fromHex(text.toUtf8());
    }
    case 1: {
        // ASCII文本模式
        return m_inputEdit->toPlainText().toUtf8();
    }
    case 2: {
        // 二进制文件模式：输入框内容为文件路径
        QString path = m_inputEdit->toPlainText().trimmed();
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            return file.readAll();
        }
        return QByteArray();
    }
    default:
        return QByteArray();
    }
}

/**
 * @brief 获取当前选中算法
 */
ChecksumCalculator::Algorithm ChecksumPanel::selectedAlgorithm() const
{
    return static_cast<ChecksumCalculator::Algorithm>(
        m_algoCombo->currentData().toInt());
}

/**
 * @brief 返回最近一次计算结果
 */
quint64 ChecksumPanel::result() const
{
    return m_result;
}

/**
 * @brief 执行校验和计算
 */
void ChecksumPanel::onCalculate()
{
    QByteArray data = inputData();
    if (data.isEmpty()) {
        m_resultLabel->setText(tr("结果：无数据"));
        m_copyBtn->setEnabled(false);
        return;
    }

    auto alg = selectedAlgorithm();
    m_result = m_calculator.calculate(data, alg);
    QString name = ChecksumCalculator::algorithmName(alg);

    // 根据结果位宽选择合适的显示格式
    QString hexResult;
    if (m_result <= 0xFF) {
        hexResult = QStringLiteral("%1").arg(m_result, 2, 16, QLatin1Char('0'));
    } else if (m_result <= 0xFFFF) {
        hexResult = QStringLiteral("%1").arg(m_result, 4, 16, QLatin1Char('0'));
    } else {
        hexResult = QStringLiteral("%1").arg(m_result, 8, 16, QLatin1Char('0'));
    }
    hexResult = hexResult.toUpper();

    m_resultLabel->setText(tr("结果：0x%1 (%2) [%3]")
                               .arg(hexResult)
                               .arg(m_result)
                               .arg(name));

    m_copyBtn->setEnabled(true);
    emit calculated(m_result, name);
}

/**
 * @brief 复制校验和结果到剪贴板
 */
void ChecksumPanel::onCopyResult()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(m_result, 16).toUpper());
}
