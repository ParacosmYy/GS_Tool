#include "h32087/m32087.h"
QVector<double> m32087::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
