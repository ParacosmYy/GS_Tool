/**
 * @file DeviceProfilePanelActions.cpp
 * @brief 设备配置面板 — CRUD操作、导入导出与列表刷新实现
 *
 * 从 DeviceProfilePanel.cpp 拆分而来，包含:
 *   - onSelectionChanged(): 选择变更处理
 *   - onNewProfile(): 新建设备配置
 *   - onEditProfile(): 编辑配置名称
 *   - onDeleteProfile(): 删除选中配置
 *   - onImportProfiles(): 从JSON文件导入
 *   - onExportProfiles(): 导出到JSON文件
 *   - refreshList(): 从注册表刷新列表
 *
 * UI构建和公共接口见 DeviceProfilePanel.cpp。
 */

#include "core/device/DeviceProfilePanel.h"

/** @brief 处理列表选择变更，更新按钮启用状态并发射profileSelected信号 */
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

/** @brief 新建设备配置，通过输入对话框获取名称后添加到注册表 */
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

/** @brief 编辑选中的配置名称，先删除旧配置再添加重命名后的配置 */
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

/** @brief 删除当前选中的设备配置 */
void DeviceProfilePanel::onDeleteProfile()
{
    if (!m_registry) { return; }

    auto items = m_profileList->selectedItems();
    if (!items.isEmpty()) {
        m_registry->removeProfile(items.first()->text());
    }
}

/** @brief 从JSON文件导入设备配置，通过文件对话框选择文件 */
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

/** @brief 将设备配置导出到JSON文件，通过文件对话框选择保存路径 */
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

/** @brief 从注册表重新加载并刷新设备列表 */
void DeviceProfilePanel::refreshList()
{
    m_profileList->clear();

    if (!m_registry) { return; }

    for (const auto &profile : m_registry->profiles()) {
        m_profileList->addItem(profile.name);
    }
}
