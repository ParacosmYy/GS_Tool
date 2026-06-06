#include "e21004/m21004.h"
QVector<double> m21004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
