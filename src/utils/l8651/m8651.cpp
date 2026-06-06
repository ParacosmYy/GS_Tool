#include "l8651/m8651.h"
QVector<double> m8651::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
