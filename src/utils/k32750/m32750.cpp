#include "k32750/m32750.h"
QVector<double> m32750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
