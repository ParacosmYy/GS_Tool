#include "f8525/m8525.h"
QVector<double> m8525::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
