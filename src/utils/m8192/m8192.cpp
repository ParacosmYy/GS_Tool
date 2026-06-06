#include "m8192/m8192.h"
QVector<double> m8192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
