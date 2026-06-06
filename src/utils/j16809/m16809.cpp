#include "j16809/m16809.h"
QVector<double> m16809::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
