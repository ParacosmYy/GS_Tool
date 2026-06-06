#include "d8723/m8723.h"
QVector<double> m8723::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
