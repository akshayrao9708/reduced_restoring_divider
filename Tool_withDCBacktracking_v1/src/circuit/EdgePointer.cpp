#include "EdgePointer.h"
#include "Circuit.h"
#include "Node.h"
#include "Edge.h"

namespace vp
{
	EdgePointer::EdgePointer(EdgeItr i) : it(i) {}
	EdgePointer::EdgePointer() {}
	Edge& EdgePointer::operator*() const { return it->second; }
	Edge* EdgePointer::operator->() const { return &it->second; }
}
