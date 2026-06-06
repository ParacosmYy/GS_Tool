#include "k32070/m32070.h"
QVector<double> m32070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
