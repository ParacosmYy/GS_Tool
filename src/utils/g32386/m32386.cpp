#include "g32386/m32386.h"
QVector<double> m32386::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
