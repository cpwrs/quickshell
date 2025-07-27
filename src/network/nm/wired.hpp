
#include <qtmetamacros.h>

namespace qs::network {

class NMWiredAdapter: public QObject{
	Q_OBJECT;

public:
	explicit NMWiredAdapter(const QString& path, QObject* parent = nullptr);

	[[nodiscard]] bool isValid() const;
	[[nodiscard]] QString path() const;
	[[nodiscard]] QString address() const;
};

} // qs::network
