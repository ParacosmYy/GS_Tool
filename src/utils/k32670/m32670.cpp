#include "k32670/m32670.h"
QVector<double> m32670::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
