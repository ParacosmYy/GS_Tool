#include "g32186/m32186.h"
QVector<double> m32186::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
