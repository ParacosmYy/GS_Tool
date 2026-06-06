#include "s16858/m16858.h"
QVector<double> m16858::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
