#include "core/layout/LayoutManager.h"
#include <QJsonDocument>
#include <QJsonObject>

LayoutManager::LayoutManager(QObject *parent) : QObject(parent) {}
LayoutManager::~LayoutManager() = default;

void LayoutManager::saveLayout(const QString &name) {
    m_savedLayouts[name] = m_items;
    m_currentLayout = name;
    emit layoutSaved(name);
}

void LayoutManager::loadLayout(const QString &name) {
    auto it = m_savedLayouts.constFind(name);
    if (it != m_savedLayouts.constEnd()) {
        m_items = it.value();
        m_currentLayout = name;
        emit layoutLoaded(name);
    }
}

void LayoutManager::deleteLayout(const QString &name) {
    m_savedLayouts.remove(name);
    if (m_currentLayout == name) m_currentLayout.clear();
    emit layoutDeleted(name);
}

void LayoutManager::setCurrentItem(const QString &id, const LayoutItem &item) {
    m_items[id] = item;
    emit itemChanged(id);
}

LayoutManager::LayoutItem LayoutManager::item(const QString &id) const {
    return m_items.value(id);
}

QStringList LayoutManager::layouts() const { return m_savedLayouts.keys(); }
QStringList LayoutManager::currentItemIds() const { return m_items.keys(); }

QJsonArray LayoutManager::serialize() const {
    QJsonArray arr;
    for (auto it = m_items.constBegin(); it != m_items.constEnd(); ++it) {
        QJsonObject obj;
        obj["id"] = it.key();
        obj["widgetType"] = it->widgetType;
        obj["x"] = it->geometry.x();
        obj["y"] = it->geometry.y();
        obj["w"] = it->geometry.width();
        obj["h"] = it->geometry.height();
        obj["visible"] = it->visible;
        obj["tabIndex"] = it->tabIndex;
        obj["parentPanel"] = it->parentPanel;
        arr.append(obj);
    }
    return arr;
}

void LayoutManager::deserialize(const QJsonArray &arr) {
    m_items.clear();
    for (const auto &val : arr) {
        auto obj = val.toObject();
        LayoutItem item;
        item.id = obj["id"].toString();
        item.widgetType = obj["widgetType"].toString();
        item.geometry = QRect(obj["x"].toInt(), obj["y"].toInt(),
                              obj["w"].toInt(), obj["h"].toInt());
        item.visible = obj["visible"].toBool(true);
        item.tabIndex = obj["tabIndex"].toInt(-1);
        item.parentPanel = obj["parentPanel"].toString();
        m_items[item.id] = item;
    }
}
