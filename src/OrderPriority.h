#include "Order.h"

class OrderPriority
{
public:

	bool operator()(Order& u, Order& v) {
		return u.getPriority() > v.getPriority();
	}
};
