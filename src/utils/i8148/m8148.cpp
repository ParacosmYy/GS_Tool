#include "i8148/m8148.h"
QVector<double> m8148::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
