#include "m19192/m19192.h"
QVector<double> m19192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
