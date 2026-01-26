#pragma once

#include <qobject.h>
#include <qqmlintegration.h>

namespace qs::network {

///! A NetworkManager connection settings profile.
class NMConnection: public QObject {
	Q_OBJECT;
	QML_ELEMENT;
	QML_UNCREATABLE("");

public:
	explicit NMConnection(QObject* parent = nullptr);

	/// Get the settings maps describing this network configuration.
	/// This will never include any secrets required for connection to the network.
	// Q_INVOKABLE QMap<QString, QVariantMap> getSettings();
	/// Get the secrets belonging to this network configuration.
	/// Only secrets from persistent storage or a running secret agent will be returned.
	/// The user will never be prompted for secrets as a result of this request.
	// Q_INVOKABLE QMap<QString, QVariantMap> getSecrets();
	/// Update the connection with new settings and properties and save the connection to disk.
	/// Secrets may be a part of the update request, and will be stored in persistent storage
	/// depending on the presence of a secret agent and the flags associated with each secret.
	// Q_INVOKABLE void updateSettings(QMap<QString, QVariantMap>);
};

} // namespace qs::network
