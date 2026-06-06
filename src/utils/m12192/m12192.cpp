#include "m12192/m12192.h"
QVector<double> m12192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
