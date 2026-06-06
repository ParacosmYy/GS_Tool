#include "e15004/m15004.h"
QVector<double> m15004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
