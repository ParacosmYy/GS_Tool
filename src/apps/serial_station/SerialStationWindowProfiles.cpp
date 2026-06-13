#include "apps/serial_station/SerialStationWindow.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QFileDialog>

#include "apps/serial_station/SerialStationController.h"
#include "apps/serial_station/services/SerialProfileCatalogService.h"
#include "apps/serial_station/ui/SerialLogPanel.h"

namespace serial_station {

namespace {

QString defaultProfileDirectoryFallback()
{
    const QString documents =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return documents.isEmpty() ? QDir::homePath() : documents;
}

QString normalizedDirectoryPath(const QString& directoryPath)
{
    const QString trimmed = directoryPath.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    return QDir::cleanPath(trimmed);
}

QString normalizedProfilePath(const QString& filePath)
{
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.suffix().isEmpty()) {
        return filePath;
    }

    return filePath + QStringLiteral(".edserialprofile");
}

} // namespace

SerialProfileWriteResult SerialStationWindow::saveCurrentProfileToFile(
    const QString& filePath,
    const QString& name,
    const QString& description,
    const QStringList& tags)
{
    const SerialStationProfile profile = collectCurrentProfile(name, description, tags);
    const SerialProfileWriteResult result = m_controller->saveProfileToFile(profile, filePath);
    if (result.ok) {
        recordSuccessfulProfilePath(result.filePath);
        setDefaultProfileDirectory(QFileInfo(result.filePath).absolutePath());
        m_logPanel->appendSystem(tr("已保存配置档案: %1").arg(result.filePath));
    } else {
        m_logPanel->appendSystem(tr("保存配置档案失败: %1").arg(result.errorMessage));
    }
    return result;
}

SerialProfileResult SerialStationWindow::loadProfileFromFile(const QString& filePath)
{
    const SerialProfileResult result = m_controller->loadProfileFromFile(filePath);
    if (!result.ok) {
        m_logPanel->appendSystem(tr("加载配置档案失败: %1").arg(result.errorMessage));
        return result;
    }

    applyProfileToUi(result.profile);
    recordSuccessfulProfilePath(filePath);
    setDefaultProfileDirectory(QFileInfo(filePath).absolutePath());
    m_logPanel->appendSystem(tr("已加载配置档案: %1").arg(result.profile.name));
    return result;
}

void SerialStationWindow::setDefaultProfileDirectory(const QString& directoryPath)
{
    const QString normalizedPath = normalizedDirectoryPath(directoryPath);
    if (normalizedPath.isEmpty()) {
        m_profileCatalog->clearDefaultProfileDirectory();
        m_defaultProfileDirectory.clear();
        return;
    }

    if (m_profileCatalog->setDefaultProfileDirectory(normalizedPath)) {
        m_defaultProfileDirectory = m_profileCatalog->defaultProfileDirectory();
    }
}

QString SerialStationWindow::defaultProfileDirectory() const
{
    if (!m_defaultProfileDirectory.isEmpty()) {
        return m_defaultProfileDirectory;
    }

    return defaultProfileDirectoryFallback();
}

void SerialStationWindow::saveProfileWithDialog()
{
    const QString defaultPath = QDir(defaultProfileDirectory())
        .filePath(QStringLiteral("serial-station.edserialprofile"));
    const QString selectedPath = QFileDialog::getSaveFileName(
        this,
        tr("保存串口工站档案"),
        defaultPath,
        tr("Serial Station Profile (*.edserialprofile);;JSON (*.json)"));

    if (selectedPath.trimmed().isEmpty()) {
        m_logPanel->appendSystem(tr("配置档案保存已取消"));
        return;
    }

    const QString profileName = QFileInfo(selectedPath).completeBaseName();
    saveCurrentProfileToFile(normalizedProfilePath(selectedPath), profileName);
}

void SerialStationWindow::loadProfileWithDialog()
{
    const QString selectedPath = QFileDialog::getOpenFileName(
        this,
        tr("加载串口工站档案"),
        defaultProfileDirectory(),
        tr("Serial Station Profile (*.edserialprofile);;JSON (*.json);;All Files (*.*)"));

    if (selectedPath.trimmed().isEmpty()) {
        m_logPanel->appendSystem(tr("配置档案加载已取消"));
        return;
    }

    loadProfileFromFile(selectedPath);
}

} // namespace serial_station
