#include "p32015/m32015.h"
QVector<double> m32015::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
