#include "BackgroundSettingsPopup.h"
#include "BackgroundWidget.h"
#include "utils/SettingsManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

BackgroundSettingsPopup::BackgroundSettingsPopup(BackgroundWidget* bgWidget, QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
    , m_bgWidget(bgWidget)
{
    setObjectName("bgSettingsPopup");
    setFixedSize(280, 280);
    setAttribute(Qt::WA_TranslucentBackground, false);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // ---- 模糊半径 ----
    auto* blurLayout = new QHBoxLayout;
    auto* blurLbl = new QLabel(tr("磨砂模糊:"), this);
    blurLbl->setFixedWidth(70);
    m_blurSlider = new QSlider(Qt::Horizontal, this);
    m_blurSlider->setObjectName("bgBlurSlider");
    m_blurSlider->setRange(0, 30);
    m_blurSlider->setValue(int(m_bgWidget->blurRadius()));
    m_blurValueLbl = new QLabel(QString::number(int(m_bgWidget->blurRadius())), this);
    m_blurValueLbl->setFixedWidth(28);
    blurLayout->addWidget(blurLbl);
    blurLayout->addWidget(m_blurSlider, 1);
    blurLayout->addWidget(m_blurValueLbl);
    mainLayout->addLayout(blurLayout);

    connect(m_blurSlider, &QSlider::valueChanged, this, [this](int val) {
        m_bgWidget->setBlurRadius(val);
        m_blurValueLbl->setText(QString::number(val));
    });

    // ---- 透明度 ----
    auto* opacityLayout = new QHBoxLayout;
    auto* opacityLbl = new QLabel(tr("背景透明度:"), this);
    opacityLbl->setFixedWidth(70);
    m_opacitySlider = new QSlider(Qt::Horizontal, this);
    m_opacitySlider->setObjectName("bgOpacitySlider");
    m_opacitySlider->setRange(0, 100);
    m_opacitySlider->setValue(int(m_bgWidget->bgOpacity() * 100));
    m_opacityValueLbl = new QLabel(QString::number(int(m_bgWidget->bgOpacity() * 100)) + "%", this);
    m_opacityValueLbl->setFixedWidth(36);
    opacityLayout->addWidget(opacityLbl);
    opacityLayout->addWidget(m_opacitySlider, 1);
    opacityLayout->addWidget(m_opacityValueLbl);
    mainLayout->addLayout(opacityLayout);

    connect(m_opacitySlider, &QSlider::valueChanged, this, [this](int val) {
        m_bgWidget->setBgOpacity(val / 100.0);
        m_opacityValueLbl->setText(QString::number(val) + "%");
    });

    // ---- 涟漪特效开关 ----
    auto* rippleCheck = new QCheckBox(tr("点击涟漪特效"), this);
    rippleCheck->setChecked(m_bgWidget->rippleEnabled());
    mainLayout->addWidget(rippleCheck);

    connect(rippleCheck, &QCheckBox::toggled, m_bgWidget, &BackgroundWidget::setRippleEnabled);

    // ---- 分隔线 ----
    auto* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator);

    // ---- 选择背景图按钮 ----
    m_selectImageBtn = new QPushButton(tr("选择背景图..."), this);
    m_selectImageBtn->setObjectName("bgSelectImageBtn");
    mainLayout->addWidget(m_selectImageBtn);

    connect(m_selectImageBtn, &QPushButton::clicked, this, &BackgroundSettingsPopup::onSelectBackground);

    // ---- 恢复默认按钮 ----
    m_resetBtn = new QPushButton(tr("恢复默认背景"), this);
    m_resetBtn->setObjectName("bgResetBtn");
    mainLayout->addWidget(m_resetBtn);

    connect(m_resetBtn, &QPushButton::clicked, this, [this]() {
        m_bgWidget->resetToDefault();
        // 清除用户保存的自定义背景路径
        SettingsManager::instance().remove("background/customImagePath");
        SettingsManager::instance().sync();
        emit resetToDefaultRequested();
    });
}

void BackgroundSettingsPopup::onSelectBackground()
{
    // 打开文件对话框，筛选图片格式
    QString lastDir = SettingsManager::instance().get(
        "background/lastOpenDir",
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
    ).toString();

    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("选择背景图片"),
        lastDir,
        tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.webp *.gif)")
    );

    if (filePath.isEmpty()) return;

    // 记住最后打开的目录
    QFileInfo fi(filePath);
    SettingsManager::instance().set("background/lastOpenDir", fi.absolutePath());

    // 设置背景图
    m_bgWidget->setBackgroundImage(filePath);

    // 保存用户选择的图片路径到设置，下次启动自动加载
    SettingsManager::instance().set("background/customImagePath", filePath);
    SettingsManager::instance().sync();

    emit backgroundImageSelected(filePath);
}

void BackgroundSettingsPopup::syncFromWidget()
{
    m_blurSlider->setValue(int(m_bgWidget->blurRadius()));
    m_blurValueLbl->setText(QString::number(int(m_bgWidget->blurRadius())));
    m_opacitySlider->setValue(int(m_bgWidget->bgOpacity() * 100));
    m_opacityValueLbl->setText(QString::number(int(m_bgWidget->bgOpacity() * 100)) + "%");
}

void BackgroundSettingsPopup::hideEvent(QHideEvent* event)
{
    emit hidden();
    QWidget::hideEvent(event);
}
