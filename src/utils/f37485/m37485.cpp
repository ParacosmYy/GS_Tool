#include "f37485/m37485.h"
QVector<double> m37485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
