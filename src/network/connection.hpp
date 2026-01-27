#pragma once

#include <qobject.h>
#include <qproperty.h>
#include <qqmlintegration.h>
#include <qtypes.h>

#include "nm/types.hpp"

namespace qs::network {

///! A NetworkManager connection settings profile.
class NMConnection: public QObject {
	Q_OBJECT;
	QML_ELEMENT;
	QML_UNCREATABLE("");

	// clang-format off
	/// A settings map describing this network configuration, including secrets.
	Q_PROPERTY(ConnectionSettingsMap settings READ settings WRITE writeSettings NOTIFY settingsChanged);
	// clang-format on

public:
	explicit NMConnection(QObject* parent = nullptr);
	/// Clear the secrets belonging to this connection.
	Q_INVOKABLE void clearSecrets();
	/// Delete the connection.
	Q_INVOKABLE void forget();

	QBindable<ConnectionSettingsMap> bindableSettings() { return &this->bSettings; }
	QBindable<ConnectionSettingsMap> bindableSecrets() { return &this->bSecrets; }

signals:
	void requestUpdateSettings(ConnectionSettingsMap settings);
	void requestClearSecrets();
	void requestForget();
	void settingsChanged();
	void secretsChanged();

private:
	// clang-format off
	Q_OBJECT_BINDABLE_PROPERTY(NMConnection, ConnectionSettingsMap, bSettings, &NMConnection::settingsChanged);
	Q_OBJECT_BINDABLE_PROPERTY(NMConnection, ConnectionSettingsMap, bSecrets, &NMConnection::secretsChanged);
	// clang-format on
};
} // namespace qs::network
