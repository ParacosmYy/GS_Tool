#include "g32586/m32586.h"
QVector<double> m32586::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
