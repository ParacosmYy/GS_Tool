#include "s16958/m16958.h"
QVector<double> m16958::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
