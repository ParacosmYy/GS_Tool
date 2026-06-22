/**
 * @file IPanelProvider.h
 * @brief 面板提供者接口
 */
#ifndef IPANEL_PROVIDER_H
#define IPANEL_PROVIDER_H
#include <QList>
#include <QString>
#include <QWidget>
struct PanelDescriptor { QString id; QString displayName; QString icon; QString category; };
class IPanelProvider {
public:
    virtual ~IPanelProvider() = default;
    virtual QString providerName() const = 0;
    virtual QList<PanelDescriptor> panelDescriptors() const = 0;
    virtual QWidget* createPanel(const QString& panelId, QWidget* parent) = 0;
};
#endif
