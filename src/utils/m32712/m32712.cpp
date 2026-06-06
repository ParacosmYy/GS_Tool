#include "m32712/m32712.h"
QVector<double> m32712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
