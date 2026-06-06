#include "f8485/m8485.h"
QVector<double> m8485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
