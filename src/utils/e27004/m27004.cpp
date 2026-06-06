#include "e27004/m27004.h"
QVector<double> m27004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
