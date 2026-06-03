/**
 * @file DeviceProfilePanel.cpp
 * @brief 设备配置选择面板 UI 实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "core/device/DeviceProfilePanel.h"

/**
 * @brief 构造函数，初始化设备选择面板布局
 */
DeviceProfilePanel::DeviceProfilePanel(QWidget *parent)
    : QWidget(parent)
    , m_profileList(new QListWidget(this))
    , m_newBtn(new QPushButton(tr("新建"), this))
    , m_editBtn(new QPushButton(tr("编辑"), this))
    , m_deleteBtn(new QPushButton(tr("删除"), this))
    , m_importBtn(new QPushButton(tr("导入"), this))
    , m_exportBtn(new QPushButton(tr("导出"), this))
{
    setObjectName(QStringLiteral("DeviceProfilePanel"));

    m_profileList->setObjectName("deviceProfileList");
    m_newBtn->setObjectName("deviceNewBtn");
    m_editBtn->setObjectName("deviceEditBtn");
    m_deleteBtn->setObjectName("deviceDeleteBtn");
    m_importBtn->setObjectName("deviceImportBtn");
    m_exportBtn->setObjectName("deviceExportBtn");

    auto *mainLayout = new QVBoxLayout(this);

    // 按钮行1：CRUD操作
    auto *crudLayout = new QHBoxLayout();
    crudLayout->addWidget(m_newBtn);
    crudLayout->addWidget(m_editBtn);
    crudLayout->addWidget(m_deleteBtn);
    crudLayout->addStretch();

    // 按钮行2：导入导出
    auto *ioLayout = new QHBoxLayout();
    ioLayout->addWidget(m_importBtn);
    ioLayout->addWidget(m_exportBtn);
    ioLayout->addStretch();

    m_editBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);

    // 设备列表
    m_profileList->setSelectionMode(QAbstractItemView::SingleSelection);

    mainLayout->addLayout(crudLayout);
    mainLayout->addLayout(ioLayout);
    mainLayout->addWidget(m_profileList);

    // 连接信号
    connect(m_profileList, &QListWidget::itemSelectionChanged,
            this, &DeviceProfilePanel::onSelectionChanged);
    connect(m_profileList, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem *item) {
        if (item && m_registry) {
            emit profileSelected(
                m_registry->findProfile(item->text()));
        }
    });
    connect(m_newBtn, &QPushButton::clicked,
            this, &DeviceProfilePanel::onNewProfile);
    connect(m_editBtn, &QPushButton::clicked,
            this, &DeviceProfilePanel::onEditProfile);
    connect(m_deleteBtn, &QPushButton::clicked,
            this, &DeviceProfilePanel::onDeleteProfile);
    connect(m_importBtn, &QPushButton::clicked,
            this, &DeviceProfilePanel::onImportProfiles);
    connect(m_exportBtn, &QPushButton::clicked,
            this, &DeviceProfilePanel::onExportProfiles);
}

/**
 * @brief 设置关联的注册表并刷新列表
 */
void DeviceProfilePanel::setRegistry(DeviceRegistry *registry)
{
    m_registry = registry;

    if (m_registry) {
        connect(m_registry, &DeviceRegistry::profilesChanged,
                this, &DeviceProfilePanel::refreshList);
        refreshList();
    }
}

/**
 * @brief 获取当前选中的设备配置
 */
DeviceProfile DeviceProfilePanel::selectedProfile() const
{
    if (!m_registry) { return {}; }

    auto items = m_profileList->selectedItems();
    if (items.isEmpty()) { return {}; }

    return m_registry->findProfile(items.first()->text());
}

/**
 * @brief 处理列表选择变更
 */
void DeviceProfilePanel::onSelectionChanged()
{
    bool hasSelection = !m_profileList->selectedItems().isEmpty();
    m_editBtn->setEnabled(hasSelection);
    m_deleteBtn->setEnabled(hasSelection);

    if (hasSelection) {
        ++m_totalSelections;
        emit profileSelected(selectedProfile());
    }
}

/**
 * @brief 新建设备配置 — 通过输入对话框获取名称
 */
void DeviceProfilePanel::onNewProfile()
{
    if (!m_registry) { return; }

    QString name = QInputDialog::getText(
        this, tr("新建设备配置"), tr("设备名称:"));
    if (name.isEmpty()) { return; }

    // 检查重名
    if (m_registry->findProfile(name).name == name) {
        return; // 已存在
    }

    DeviceProfile profile = DeviceProfile::createDefault();
    profile.name = name;
    m_registry->addProfile(profile);
}

/**
 * @brief 编辑选中的配置 — 修改名称
 */
void DeviceProfilePanel::onEditProfile()
{
    if (!m_registry) { return; }

    auto items = m_profileList->selectedItems();
    if (items.isEmpty()) { return; }

    QString oldName = items.first()->text();
    DeviceProfile profile = m_registry->findProfile(oldName);
    if (profile.name.isEmpty()) { return; }

    QString newName = QInputDialog::getText(
        this, tr("编辑设备配置"), tr("设备名称:"),
        QLineEdit::Normal, oldName);
    if (newName.isEmpty() || newName == oldName) { return; }

    // 更新名称：删除旧的，添加新的
    m_registry->removeProfile(oldName);
    profile.name = newName;
    m_registry->addProfile(profile);
    ++m_totalProfileEdits;
}

/**
 * @brief 删除选中的配置
 */
void DeviceProfilePanel::onDeleteProfile()
{
    if (!m_registry) { return; }

    auto items = m_profileList->selectedItems();
    if (!items.isEmpty()) {
        m_registry->removeProfile(items.first()->text());
    }
}

/**
 * @brief 从文件导入配置
 */
void DeviceProfilePanel::onImportProfiles()
{
    if (!m_registry) { return; }

    QString path = QFileDialog::getOpenFileName(
        this, tr("导入设备配置"), QString(),
        tr("JSON文件 (*.json);;所有文件 (*)"));
    if (path.isEmpty()) { return; }

    m_registry->loadFromFile(path);
    ++m_totalImports;
}

/**
 * @brief 导出配置到文件
 */
void DeviceProfilePanel::onExportProfiles()
{
    if (!m_registry) { return; }

    QString path = QFileDialog::getSaveFileName(
        this, tr("导出设备配置"), QString(),
        tr("JSON文件 (*.json);;所有文件 (*)"));
    if (path.isEmpty()) { return; }

    m_registry->saveToFile(path);
    ++m_totalExports;
}

/**
 * @brief 刷新设备列表
 */
void DeviceProfilePanel::refreshList()
{
    m_profileList->clear();

    if (!m_registry) { return; }

    for (const auto &profile : m_registry->profiles()) {
        m_profileList->addItem(profile.name);
    }
}
