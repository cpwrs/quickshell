#include "network.hpp"
#include <utility>

#include <qlogging.h>
#include <qloggingcategory.h>
#include <qobject.h>
#include <qstring.h>
#include <qtmetamacros.h>
#include <qdebug.h>

#include "../core/logcat.hpp"
#include "device.hpp"
#include "enums.hpp"
#include "nm/backend.hpp"
#include "nm/types.hpp"

namespace qs::network {

namespace {
QS_LOGGING_CATEGORY(logNetwork, "quickshell.network", QtWarningMsg);
QS_LOGGING_CATEGORY(logConnection, "quickshell.network.connection", QtWarningMsg);

bool wpaPskIsValid(const QString& psk) {
	if (psk.isEmpty()) return false;
	const auto psklen = psk.length();

	// ASCII passphrase
	if (psklen < 8 || psklen > 64) return false;

	// Hex PSK
	if (psklen == 64) {
		for (int i = 0; i < psklen; ++i) {
			if (!psk.at(i).isLetterOrNumber()) {
				return false;
			}
		}
	}

	return true;
}

} // namespace

Networking::Networking(QObject* parent): QObject(parent) {
	// Try to create the NetworkManager backend and bind to it.
	auto* nm = new NetworkManager(this);
	if (nm->isAvailable()) {
		QObject::connect(nm, &NetworkManager::deviceAdded, this, &Networking::deviceAdded);
		QObject::connect(nm, &NetworkManager::deviceRemoved, this, &Networking::deviceRemoved);
		QObject::connect(this, &Networking::requestSetWifiEnabled, nm, &NetworkManager::setWifiEnabled);
		this->bindableWifiEnabled().setBinding([nm]() { return nm->wifiEnabled(); });
		this->bindableWifiHardwareEnabled().setBinding([nm]() { return nm->wifiHardwareEnabled(); });

		this->mBackend = nm;
		this->mBackendType = NetworkBackendType::NetworkManager;
		return;
	} else {
		delete nm;
	}

	qCCritical(logNetwork) << "Network will not work. Could not find an available backend.";
}

void Networking::deviceAdded(NetworkDevice* dev) { this->mDevices.insertObject(dev); }
void Networking::deviceRemoved(NetworkDevice* dev) { this->mDevices.removeObject(dev); }

void Networking::setWifiEnabled(bool enabled) {
	if (this->bWifiEnabled == enabled) return;
	emit this->requestSetWifiEnabled(enabled);
}

NMConnection::NMConnection(QObject* parent): QObject(parent) {}

void NMConnection::updateSettings(const ConnectionSettingsMap& settings) {
	emit this->requestUpdateSettings(settings);
}

void NMConnection::clearSecrets() { emit this->requestClearSecrets(); }

void NMConnection::forget() { emit this->requestForget(); }

void NMConnection::setWifiPsk(const QString& psk) {
	if (this->bWifiSecurity != WifiSecurityType::WpaPsk
	    && this->bWifiSecurity != WifiSecurityType::Wpa2Psk)
	{
		return;
	}
	if (!wpaPskIsValid(psk)) {
		qCWarning(logConnection) << "Malformed PSK provided to" << this;
	}
	emit this->requestSetWifiPsk(psk);
}

Network::Network(QString name, QObject* parent): QObject(parent), mName(std::move(name)) {
	this->bStateChanging.setBinding([this] {
		auto state = this->bState.value();
		return state == NetworkState::Connecting || state == NetworkState::Disconnecting;
	});
};

void Network::setNmDefaultConnection(NMConnection* conn) {
	if (this->bNmDefaultConnection == conn) return;
	if (!this->mNmConnections.valueList().contains(conn)) return;
	emit this->requestSetNmDefaultConnection(conn);
}

void Network::connect() {
	if (this->bConnected) {
		qCCritical(logNetwork) << this << "is already connected.";
		return;
	}

	this->requestConnect();
}

void Network::disconnect() {
	if (!this->bConnected) {
		qCCritical(logNetwork) << this << "is not currently connected";
		return;
	}

	this->requestDisconnect();
}

void Network::forget() { this->requestForget(); }

void Network::connectionAdded(NMConnection* conn) { this->mNmConnections.insertObject(conn); }
void Network::connectionRemoved(NMConnection* conn) { this->mNmConnections.removeObject(conn); }

NMConnectionContext::NMConnectionContext(QObject* parent): QObject(parent) {}

void NMConnectionContext::setNetwork(Network* network) {
	if (this->bNetwork == network) return;
	if (this->bNetwork) disconnect(this->bNetwork, nullptr, this, nullptr);
	this->bNetwork = network;

	connect(network, &Network::stateChanged, this, [network, this]() {
		if (network->state() == NetworkState::Connected) emit this->success();
	});
	connect(network, &Network::stateReasonChanged, this, [network, this]() {
		if (network->stateReason() == NMNetworkStateReason::NoSecrets) emit this->noSecrets();
		if (network->stateReason() == NMNetworkStateReason::LoginFailed) emit this->loginFailed();
	});
	connect(network, &Network::destroyed, this, [this]() { this->bNetwork = nullptr; });
}

} // namespace qs::network

QDebug operator<<(QDebug debug, const qs::network::NMConnection* connection) {
	auto saver = QDebugStateSaver(debug);

	if (connection) {
		debug.nospace() << "NMConnection(" << static_cast<const void*>(connection)
		                << ", id=" << connection->id() << ")";
	} else {
		debug << "NMConnection(nullptr)";
	}

	return debug;
}
