#include "j32629/m32629.h"
QVector<double> m32629::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
