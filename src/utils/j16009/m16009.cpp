#include "j16009/m16009.h"
QVector<double> m16009::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
