#include "connection.hpp"

namespace qs::network {

NMConnection::NMConnection(QObject* parent): QObject(parent) {}

void NMConnection::clearSecrets() {
	emit this->requestClearSecrets();
}

void NMConnection::forget() {
	emit this->requestForget();
}

} // namespace qs::network
