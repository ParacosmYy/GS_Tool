#include "k32090/m32090.h"
QVector<double> m32090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
