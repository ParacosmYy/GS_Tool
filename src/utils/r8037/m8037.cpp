#include "r8037/m8037.h"
QVector<double> m8037::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
