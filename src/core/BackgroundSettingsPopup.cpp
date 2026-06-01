/**
 * @file BackgroundSettingsPopup.cpp
 * @brief 背景设置弹出面板实现 - 磨砂玻璃/透明度/涟漪开关/自定义背景图的实时调节
 *
 * 作为浮动弹出窗口（Qt::Popup），点击外部区域自动关闭。
 * 所有调节实时反映到 BackgroundWidget。
 */

#include "core/BackgroundSettingsPopup.h"
#include "core/AnimatedButton.h"
#include "core/BackgroundWidget.h"
#include "core/Constants.h"
#include "utils/SettingsManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

/**
 * @brief 构造背景设置弹出面板
 * 创建所有 UI 控件并连接到 BackgroundWidget 的属性方法
 * @param bgWidget 被控的背景控件实例
 * @param parent 父 widget
 */
BackgroundSettingsPopup::BackgroundSettingsPopup(BackgroundWidget* bgWidget, QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)  // Popup: 点击外部关闭, Frameless: 无标题栏
    , m_bgWidget(bgWidget)
{
    setObjectName("bgSettingsPopup");
    setFixedSize(280, 280);
    setAttribute(Qt::WA_TranslucentBackground, false);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(Layout::kPanelPadding, Layout::kPanelPadding,
                                   Layout::kPanelPadding, Layout::kPanelPadding);
    mainLayout->setSpacing(Layout::kGroupSpacing);

    // 创建所有滑块、开关和分隔线控件
    createControls(mainLayout);

    // ---- 选择背景图按钮 ----
    m_selectImageBtn = new AnimatedButton(tr("选择背景图..."), this);
    m_selectImageBtn->setObjectName("bgSelectImageBtn");
    mainLayout->addWidget(m_selectImageBtn);
    connect(m_selectImageBtn, &QPushButton::clicked, this, &BackgroundSettingsPopup::onSelectBackground);

    // ---- 恢复默认按钮 ----
    m_resetBtn = new AnimatedButton(tr("恢复默认背景"), this);
    m_resetBtn->setObjectName("bgResetBtn");
    mainLayout->addWidget(m_resetBtn);
    connect(m_resetBtn, &QPushButton::clicked, this, [this]() {
        m_bgWidget->resetToDefault();
        SettingsManager::instance().remove("background/customImagePath");
        SettingsManager::instance().sync();
        emit resetToDefaultRequested();
    });
}

/** @brief 创建模糊/透明度滑块、涟漪开关、分隔线并添加到布局 */
void BackgroundSettingsPopup::createControls(QVBoxLayout* mainLayout)
{
    // ---- 模糊半径滑块 ----
    auto* blurLayout = new QHBoxLayout;
    auto* blurLbl = new QLabel(tr("磨砂模糊:"), this);
    blurLbl->setObjectName("bgBlurLabel"); blurLbl->setFixedWidth(Layout::kLabelFixedWidth);
    m_blurSlider = new QSlider(Qt::Horizontal, this);
    m_blurSlider->setObjectName("bgBlurSlider");
    m_blurSlider->setRange(0, 30); m_blurSlider->setValue(int(m_bgWidget->blurRadius()));
    m_blurValueLbl = new QLabel(QString::number(int(m_bgWidget->blurRadius())), this);
    m_blurValueLbl->setObjectName("bgBlurValueLabel"); m_blurValueLbl->setFixedWidth(28);
    blurLayout->addWidget(blurLbl); blurLayout->addWidget(m_blurSlider, 1); blurLayout->addWidget(m_blurValueLbl);
    mainLayout->addLayout(blurLayout);
    connect(m_blurSlider, &QSlider::valueChanged, this, [this](int val) {
        m_bgWidget->setBlurRadius(val); m_blurValueLbl->setText(QString::number(val));
    });

    // ---- 背景透明度滑块 ----
    auto* opacityLayout = new QHBoxLayout;
    auto* opacityLbl = new QLabel(tr("背景透明度:"), this);
    opacityLbl->setObjectName("bgOpacityLabel"); opacityLbl->setFixedWidth(Layout::kLabelFixedWidth);
    m_opacitySlider = new QSlider(Qt::Horizontal, this);
    m_opacitySlider->setObjectName("bgOpacitySlider");
    m_opacitySlider->setRange(0, 100); m_opacitySlider->setValue(int(m_bgWidget->bgOpacity() * 100));
    m_opacityValueLbl = new QLabel(QString::number(int(m_bgWidget->bgOpacity() * 100)) + "%", this);
    m_opacityValueLbl->setObjectName("bgOpacityValueLabel"); m_opacityValueLbl->setFixedWidth(Layout::kSliderValueWidth);
    opacityLayout->addWidget(opacityLbl); opacityLayout->addWidget(m_opacitySlider, 1); opacityLayout->addWidget(m_opacityValueLbl);
    mainLayout->addLayout(opacityLayout);
    connect(m_opacitySlider, &QSlider::valueChanged, this, [this](int val) {
        m_bgWidget->setBgOpacity(val / 100.0); m_opacityValueLbl->setText(QString::number(val) + "%");
    });

    // ---- 涟漪特效开关 ----
    auto* rippleCheck = new QCheckBox(tr("点击涟漪特效"), this);
    rippleCheck->setObjectName("bgRippleCheck"); rippleCheck->setChecked(m_bgWidget->rippleEnabled());
    mainLayout->addWidget(rippleCheck);
    connect(rippleCheck, &QCheckBox::toggled, m_bgWidget, &BackgroundWidget::setRippleEnabled);

    // ---- 分隔线 ----
    auto* separator = new QFrame(this);
    separator->setObjectName("bgSeparator");
    separator->setFrameShape(QFrame::HLine); separator->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator);
}

/**
 * @brief 打开文件对话框选择背景图
 * 记住上次打开的目录，选择后保存路径到 SettingsManager 以便下次启动自动加载
 */
void BackgroundSettingsPopup::onSelectBackground()
{
    // 从设置恢复上次打开的目录，默认为系统图片目录
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

/**
 * @brief 从 BackgroundWidget 同步当前值到 UI 控件
 * 每次显示弹出面板前调用，确保滑块位置与实际值一致
 */
void BackgroundSettingsPopup::syncFromWidget()
{
    m_blurSlider->setValue(int(m_bgWidget->blurRadius()));
    m_blurValueLbl->setText(QString::number(int(m_bgWidget->blurRadius())));
    m_opacitySlider->setValue(int(m_bgWidget->bgOpacity() * 100));
    m_opacityValueLbl->setText(QString::number(int(m_bgWidget->bgOpacity() * 100)) + "%");
}

/**
 * @brief 隐藏事件处理
 * 在面板隐藏时发出 hidden() 信号，通知 MainWindow 更新工具栏按钮状态
 */
void BackgroundSettingsPopup::hideEvent(QHideEvent* event)
{
    emit hidden();
    QWidget::hideEvent(event);
}
