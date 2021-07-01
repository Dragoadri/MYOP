#include "selfdrive/ui/qt/offroad/wifiManager.h"

#include <algorithm>
#include <set>
#include <cstdlib>

#include "selfdrive/common/params.h"
#include "selfdrive/common/swaglog.h"

/**
 * We are using a NetworkManager DBUS API : https://developer.gnome.org/NetworkManager/1.26/spec.html
 * */

// https://developer.gnome.org/NetworkManager/1.26/nm-dbus-types.html#NM80211ApFlags
const int NM_802_11_AP_FLAGS_NONE = 0x00000000;
const int NM_802_11_AP_FLAGS_PRIVACY = 0x00000001;
const int NM_802_11_AP_FLAGS_WPS = 0x00000002;

// https://developer.gnome.org/NetworkManager/1.26/nm-dbus-types.html#NM80211ApSecurityFlags
const int NM_802_11_AP_SEC_PAIR_WEP40      = 0x00000001;
const int NM_802_11_AP_SEC_PAIR_WEP104     = 0x00000002;
const int NM_802_11_AP_SEC_GROUP_WEP40     = 0x00000010;
const int NM_802_11_AP_SEC_GROUP_WEP104    = 0x00000020;
const int NM_802_11_AP_SEC_KEY_MGMT_PSK    = 0x00000100;
const int NM_802_11_AP_SEC_KEY_MGMT_802_1X = 0x00000200;

const QString nm_path                = "/org/freedesktop/NetworkManager";
const QString nm_settings_path       = "/org/freedesktop/NetworkManager/Settings";

const QString nm_iface               = "org.freedesktop.NetworkManager";
const QString props_iface            = "org.freedesktop.DBus.Properties";
const QString nm_settings_iface      = "org.freedesktop.NetworkManager.Settings";
const QString nm_settings_conn_iface = "org.freedesktop.NetworkManager.Settings.Connection";
const QString device_iface           = "org.freedesktop.NetworkManager.Device";
const QString wireless_device_iface  = "org.freedesktop.NetworkManager.Device.Wireless";
const QString ap_iface               = "org.freedesktop.NetworkManager.AccessPoint";
const QString connection_iface       = "org.freedesktop.NetworkManager.Connection.Active";
const QString ipv4config_iface       = "org.freedesktop.NetworkManager.IP4Config";

const QString nm_service             = "org.freedesktop.NetworkManager";

const int state_connected = 100;
const int state_need_auth = 60;
const int reason_wrong_password = 8;
const int dbus_timeout = 100;

template <typename T>
T get_response(QDBusMessage response) {
  QVariant first =  response.arguments().at(0);
  QDBusVariant dbvFirst = first.value<QDBusVariant>();
  QVariant vFirst = dbvFirst.variant();
  if (vFirst.canConvert<T>()) {
    return vFirst.value<T>();
  } else {
    LOGE("Variant unpacking failure");
    return T();
  }
}

bool compare_by_strength(const Network &a, const Network &b) {
  if (a.connected == ConnectedType::CONNECTED) return true;
  if (b.connected == ConnectedType::CONNECTED) return false;
  if (a.connected == ConnectedType::CONNECTING) return true;
  if (b.connected == ConnectedType::CONNECTING) return false;
  return a.strength > b.strength;
}

WifiManager::WifiManager(QWidget* parent) : QWidget(parent) {
  qDBusRegisterMetaType<Connection>();
  qDBusRegisterMetaType<IpConfig>();
  connecting_to_network = "";
  adapter = get_adapter();

  bool has_adapter = adapter != "";
  if (!has_adapter) {
    throw std::runtime_error("Error connecting to NetworkManager");
  }
  initActiveAp();
  initConnections();
  initNetworks();

  QDBusInterface nm(nm_service, adapter, device_iface, bus);
  bus.connect(nm_service, adapter, device_iface, "StateChanged", this, SLOT(stateChange(unsigned int, unsigned int, unsigned int)));
  bus.connect(nm_service, adapter, props_iface, "PropertiesChanged", this, SLOT(propertyChange(QString, QVariantMap, QStringList)));

  bus.connect(nm_service, adapter, wireless_device_iface, "AccessPointAdded", this, SLOT(addAccessPoint(QDBusObjectPath)));
  bus.connect(nm_service, adapter, wireless_device_iface, "AccessPointRemoved", this, SLOT(removeAccessPoint(QDBusObjectPath)));

  bus.connect(nm_service, nm_settings_path, nm_settings_iface, "ConnectionRemoved", this, SLOT(connectionRemoved(QDBusObjectPath)));
  bus.connect(nm_service, nm_settings_path, nm_settings_iface, "NewConnection", this, SLOT(newConnection(QDBusObjectPath)));

  QDBusInterface device_props(nm_service, adapter, props_iface, bus);
  device_props.setTimeout(dbus_timeout);
  QDBusMessage response = device_props.call("Get", device_iface, "State");
  raw_adapter_state = get_response<uint>(response);

  // Set tethering ssid as "weedle" + first 4 characters of a dongle id
  tethering_ssid = "weedle";
  std::string bytes = Params().get("DongleId");
  if (bytes.length() >= 4) {
    tethering_ssid+="-"+QString::fromStdString(bytes.substr(0,4));
  }

  // Create dbus interface for tethering button. This populates the introspection cache,
  // making sure all future creations are non-blocking
  // https://bugreports.qt.io/browse/QTBUG-14485
  QDBusInterface(nm_service, nm_settings_path, nm_settings_iface, bus);
}

void WifiManager::initNetworks() {
  QDBusInterface nm(nm_service, adapter, wireless_device_iface, bus);
  nm.setTimeout(dbus_timeout);

  const QDBusReply<QList<QDBusObjectPath>> response = nm.call("GetAllAccessPoints");
  for (const QDBusObjectPath &path : response.value()) {
    addAccessPoint(path);
  }
  std::sort(seenNetworks.begin(), seenNetworks.end(), compare_by_strength);
}

void WifiManager::updateNetworks() {
  for (Network &network : seenNetworks) {
    network.strength = get_ap_strength(network.path);
    network.connected = getConnectedType(network.path, network.ssid);
    network.known = isKnownConnection(network.ssid);
  }
  std::sort(seenNetworks.begin(), seenNetworks.end(), compare_by_strength);
}

QString WifiManager::get_ipv4_address() {
  if (raw_adapter_state != state_connected) {
    return "";
  }
  QVector<QDBusObjectPath> conns = get_active_connections();
  for (auto &p : conns) {
    QString active_connection = p.path();
    QDBusInterface nm(nm_service, active_connection, props_iface, bus);
    nm.setTimeout(dbus_timeout);

    QDBusObjectPath pth = get_response<QDBusObjectPath>(nm.call("Get", connection_iface, "Ip4Config"));
    QString ip4config = pth.path();

    QString type = get_response<QString>(nm.call("Get", connection_iface, "Type"));

    if (type == "802-11-wireless") {
      QDBusInterface nm2(nm_service, ip4config, props_iface, bus);
      nm2.setTimeout(dbus_timeout);

      const QDBusArgument &arr = get_response<QDBusArgument>(nm2.call("Get", ipv4config_iface, "AddressData"));
      QMap<QString, QVariant> pth2;
      arr.beginArray();
      while (!arr.atEnd()) {
        arr >> pth2;
        QString ipv4 = pth2.value("address").value<QString>();
        arr.endArray();
        return ipv4;
      }
      arr.endArray();
    }
  }
  return "";
}

SecurityType WifiManager::getSecurityType(const QString &path) {
  int sflag = get_property(path, "Flags").toInt();
  int wpaflag = get_property(path, "WpaFlags").toInt();
  int rsnflag = get_property(path, "RsnFlags").toInt();
  int wpa_props = wpaflag | rsnflag;

  // obtained by looking at flags of networks in the office as reported by an Android phone
  const int supports_wpa = NM_802_11_AP_SEC_PAIR_WEP40 | NM_802_11_AP_SEC_PAIR_WEP104 | NM_802_11_AP_SEC_GROUP_WEP40 | NM_802_11_AP_SEC_GROUP_WEP104 | NM_802_11_AP_SEC_KEY_MGMT_PSK;

  if ((sflag == NM_802_11_AP_FLAGS_NONE) || ((sflag & NM_802_11_AP_FLAGS_WPS) && !(wpa_props & supports_wpa))) {
    return SecurityType::OPEN;
  } else if ((sflag & NM_802_11_AP_FLAGS_PRIVACY) && (wpa_props & supports_wpa) && !(wpa_props & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
    return SecurityType::WPA;
  } else {
    LOGW("Unsupported network! sflag: %d, wpaflag: %d, rsnflag: %d", sflag, wpaflag, rsnflag);
    return SecurityType::UNSUPPORTED;
  }
}

ConnectedType WifiManager::getConnectedType(const QString &path, const QString &ssid) {
  if (path != activeAp) {
    return ConnectedType::DISCONNECTED;
  } else {
    if (ssid == connecting_to_network) {
      return ConnectedType::CONNECTING;
    } else {
      return ConnectedType::CONNECTED;
    }
  }
}

void WifiManager::connect(const Network &n) {
  return connect(n, "", "");
}

void WifiManager::connect(const Network &n, const QString &password) {
  return connect(n, "", password);
}

void WifiManager::connect(const Network &n, const QString &username, const QString &password) {
  connecting_to_network = n.ssid;
  // disconnect();
  forgetConnection(n.ssid);  // Clear all connections that may already exist to the network we are connecting
  connect(n.ssid, username, password, n.security_type);
}

void WifiManager::connect(const QByteArray &ssid, const QString &username, const QString &password, SecurityType security_type) {
  Connection connection;
  connection["connection"]["type"] = "802-11-wireless";
  connection["connection"]["uuid"] = QUuid::createUuid().toString().remove('{').remove('}');
  connection["connection"]["id"] = "openpilot connection "+QString::fromStdString(ssid.toStdString());
  connection["connection"]["autoconnect-retries"] = 0;

  connection["802-11-wireless"]["ssid"] = ssid;
  connection["802-11-wireless"]["mode"] = "infrastructure";

  if (security_type == SecurityType::WPA) {
    connection["802-11-wireless-security"]["key-mgmt"] = "wpa-psk";
    connection["802-11-wireless-security"]["auth-alg"] = "open";
    connection["802-11-wireless-security"]["psk"] = password;
  }

  connection["ipv4"]["method"] = "auto";
  connection["ipv6"]["method"] = "ignore";

  QDBusInterface nm_settings(nm_service, nm_settings_path, nm_settings_iface, bus);
  nm_settings.setTimeout(dbus_timeout);

  nm_settings.call("AddConnection", QVariant::fromValue(connection));
}

void WifiManager::deactivateConnection(const QString &ssid) {
  for (QDBusObjectPath active_connection_raw : get_active_connections()) {
    QString active_connection = active_connection_raw.path();
    QDBusInterface nm(nm_service, active_connection, props_iface, bus);
    nm.setTimeout(dbus_timeout);

    QDBusObjectPath pth = get_response<QDBusObjectPath>(nm.call("Get", connection_iface, "SpecificObject"));
    if (pth.path() != "" && pth.path() != "/") {
      QString Ssid = get_property(pth.path(), "Ssid");
      if (Ssid == ssid) {
        QDBusInterface nm2(nm_service, nm_path, nm_iface, bus);
        nm2.setTimeout(dbus_timeout);
        nm2.call("DeactivateConnection", QVariant::fromValue(active_connection_raw));
      }
    }
  }
}

QVector<QDBusObjectPath> WifiManager::get_active_connections() {
  QDBusInterface nm(nm_service, nm_path, props_iface, bus);
  nm.setTimeout(dbus_timeout);

  QDBusMessage response = nm.call("Get", nm_iface, "ActiveConnections");
  const QDBusArgument &arr = get_response<QDBusArgument>(response);
  QVector<QDBusObjectPath> conns;

  QDBusObjectPath path;
  arr.beginArray();
  while (!arr.atEnd()) {
    arr >> path;
    conns.push_back(path);
  }
  arr.endArray();
  return conns;
}

bool WifiManager::isKnownConnection(const QString &ssid) {
  return !getConnectionPath(ssid).path().isEmpty();
}

void WifiManager::forgetConnection(const QString &ssid) {
  const QDBusObjectPath &path = getConnectionPath(ssid);
  if (!path.path().isEmpty()) {
    QDBusInterface nm2(nm_service, path.path(), nm_settings_conn_iface, bus);
    nm2.call("Delete");
  }
}

void WifiManager::requestScan() {
  QDBusInterface nm(nm_service, adapter, wireless_device_iface, bus);
  nm.setTimeout(dbus_timeout);
  nm.call("RequestScan",  QVariantMap());
}

uint WifiManager::get_wifi_device_state() {
  QDBusInterface device_props(nm_service, adapter, props_iface, bus);
  device_props.setTimeout(dbus_timeout);

  QDBusMessage response = device_props.call("Get", device_iface, "State");
  uint resp = get_response<uint>(response);
  return resp;
}

void WifiManager::initActiveAp() {
  QDBusInterface device_props(nm_service, adapter, props_iface, bus);
  device_props.setTimeout(dbus_timeout);

  const QDBusMessage &response = device_props.call("Get", wireless_device_iface, "ActiveAccessPoint");
  activeAp = get_response<QDBusObjectPath>(response).path();
}

QByteArray WifiManager::get_property(const QString &network_path , const QString &property) {
  QDBusInterface device_props(nm_service, network_path, props_iface, bus);
  device_props.setTimeout(dbus_timeout);

  QDBusMessage response = device_props.call("Get", ap_iface, property);
  return get_response<QByteArray>(response);
}

unsigned int WifiManager::get_ap_strength(const QString &network_path) {
  QDBusInterface device_props(nm_service, network_path, props_iface, bus);
  device_props.setTimeout(dbus_timeout);

  QDBusMessage response = device_props.call("Get", ap_iface, "Strength");
  return get_response<unsigned int>(response);
}

QString WifiManager::get_adapter() {
  QDBusInterface nm(nm_service, nm_path, nm_iface, bus);
  nm.setTimeout(dbus_timeout);

  QDBusMessage response = nm.call("GetDevices");
  QVariant first =  response.arguments().at(0);

  QString adapter_path = "";

  const QDBusArgument &args = first.value<QDBusArgument>();
  args.beginArray();
  while (!args.atEnd()) {
    QDBusObjectPath path;
    args >> path;

    // Get device type
    QDBusInterface device_props(nm_service, path.path(), props_iface, bus);
    device_props.setTimeout(dbus_timeout);

    QDBusMessage response = device_props.call("Get", device_iface, "DeviceType");
    uint device_type = get_response<uint>(response);

    if (device_type == 2) { // Wireless
      adapter_path = path.path();
      break;
    }
  }
  args.endArray();

  return adapter_path;
}

void WifiManager::stateChange(unsigned int new_state, unsigned int previous_state, unsigned int change_reason) {
  raw_adapter_state = new_state;
  if (new_state == state_need_auth && change_reason == reason_wrong_password) {
    forgetConnection(connecting_to_network);
    emit wrongPassword(connecting_to_network);
  } else if (new_state == state_connected) {
    connecting_to_network = "";
  }
}

// https://developer.gnome.org/NetworkManager/stable/gdbus-org.freedesktop.NetworkManager.Device.Wireless.html
void WifiManager::propertyChange(const QString &interface, const QVariantMap &props, const QStringList &invalidated_props) {
  if (interface == wireless_device_iface && props.contains("ActiveAccessPoint")) {
    const QDBusObjectPath &path = props.value("ActiveAccessPoint").value<QDBusObjectPath>();
    activeAp = path.path();
  }
}

void WifiManager::addAccessPoint(const QDBusObjectPath &path) {
  const QByteArray &ssid = get_property(path.path(), "Ssid");
  auto it = std::find_if(seenNetworks.begin(), seenNetworks.end(), [&ssid](const Network &n) {
    return n.ssid == ssid;
  });

  if (ssid.isEmpty() || (it != seenNetworks.end())) {
    return;
  }
  const unsigned int strength = get_ap_strength(path.path());
  const SecurityType security = getSecurityType(path.path());
  const ConnectedType ctype = getConnectedType(path.path(), ssid);

  Network network = {path.path(), ssid, strength, ctype, security, isKnownConnection(ssid)};
  seenNetworks.push_back(network);
}

void WifiManager::removeAccessPoint(const QDBusObjectPath &path) {
  seenNetworks.erase(std::remove_if(seenNetworks.begin(), seenNetworks.end(), [&path](const Network &n) {
    return n.path == path.path();
  }), seenNetworks.end());
}

void WifiManager::connectionRemoved(const QDBusObjectPath &path) {
  knownConnections.remove(path);
}

void WifiManager::newConnection(const QDBusObjectPath &path) {
  knownConnections[path] = getConnectionSsid(path);
  activateWifiConnection(knownConnections[path]);
}

void WifiManager::disconnect() {
  if (activeAp != "" && activeAp != "/") {
    deactivateConnection(get_property(activeAp, "Ssid"));
  }
}

QDBusObjectPath WifiManager::getConnectionPath(const QString &ssid) {
  for (const QString &conn_ssid : knownConnections) {
    if (ssid == conn_ssid) {
      return knownConnections.key(conn_ssid);
    }
  }
  return QDBusObjectPath();  // unknown ssid, return uninitialized path
}

QString WifiManager::getConnectionSsid(const QDBusObjectPath &path) {
  QDBusInterface nm(nm_service, path.path(), nm_settings_conn_iface, bus);
  nm.setTimeout(dbus_timeout);
  const QDBusReply<Connection> result = nm.call("GetSettings");
  return result.value().value("802-11-wireless").value("ssid").toString();
}

void WifiManager::initConnections() {
  QDBusInterface nm(nm_service, nm_settings_path, nm_settings_iface, bus);
  nm.setTimeout(dbus_timeout);

  const QDBusReply<QList<QDBusObjectPath>> response = nm.call("ListConnections");
  for (const QDBusObjectPath &path : response.value()) {
    knownConnections[path] = getConnectionSsid(path);
  }
}

void WifiManager::activateWifiConnection(const QString &ssid) {
  const QDBusObjectPath &path = getConnectionPath(ssid);
  if (!path.path().isEmpty()) {
    connecting_to_network = ssid;
    QString devicePath = get_adapter();
    QDBusInterface nm3(nm_service, nm_path, nm_iface, bus);
    nm3.setTimeout(dbus_timeout);
    nm3.call("ActivateConnection", QVariant::fromValue(path), QVariant::fromValue(QDBusObjectPath(devicePath)), QVariant::fromValue(QDBusObjectPath("/")));
  }
}

// Functions for tethering
void WifiManager::addTetheringConnection() {
  Connection connection;
  connection["connection"]["id"] = "Hotspot";
  connection["connection"]["uuid"] = QUuid::createUuid().toString().remove('{').remove('}');
  connection["connection"]["type"] = "802-11-wireless";
  connection["connection"]["interface-name"] = "wlan0";
  connection["connection"]["autoconnect"] = false;

  connection["802-11-wireless"]["band"] = "bg";
  connection["802-11-wireless"]["mode"] = "ap";
  connection["802-11-wireless"]["ssid"] = tethering_ssid.toUtf8();

  connection["802-11-wireless-security"]["group"] = QStringList("ccmp");
  connection["802-11-wireless-security"]["key-mgmt"] = "wpa-psk";
  connection["802-11-wireless-security"]["pairwise"] = QStringList("ccmp");
  connection["802-11-wireless-security"]["proto"] = QStringList("rsn");
  connection["802-11-wireless-security"]["psk"] = tetheringPassword;

  connection["ipv4"]["method"] = "shared";
  QMap<QString,QVariant> address;
  address["address"] = "192.168.43.1";
  address["prefix"] = 24u;
  connection["ipv4"]["address-data"] = QVariant::fromValue(IpConfig() << address);
  connection["ipv4"]["gateway"] = "192.168.43.1";
  connection["ipv4"]["route-metric"] = 1100;
  connection["ipv6"]["method"] = "ignore";

  QDBusInterface nm_settings(nm_service, nm_settings_path, nm_settings_iface, bus);
  nm_settings.setTimeout(dbus_timeout);
  nm_settings.call("AddConnection", QVariant::fromValue(connection));
}

void WifiManager::enableTethering() {
  if (!isKnownConnection(tethering_ssid.toUtf8())) {
    addTetheringConnection();
  }
  activateWifiConnection(tethering_ssid.toUtf8());
}

void WifiManager::disableTethering() {
  deactivateConnection(tethering_ssid.toUtf8());
}

bool WifiManager::tetheringEnabled() {
  return get_property(activeAp, "Ssid") == tethering_ssid;
}

void WifiManager::changeTetheringPassword(const QString &newPassword) {
  tetheringPassword = newPassword;
  forgetConnection(tethering_ssid.toUtf8());
  addTetheringConnection();
}
