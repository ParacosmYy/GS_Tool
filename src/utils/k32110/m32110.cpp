#include "k32110/m32110.h"
QVector<double> m32110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
