#include "e25004/m25004.h"
QVector<double> m25004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
