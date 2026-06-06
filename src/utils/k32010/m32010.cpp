#include "k32010/m32010.h"
QVector<double> m32010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
