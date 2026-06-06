#include "g32926/m32926.h"
QVector<double> m32926::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
