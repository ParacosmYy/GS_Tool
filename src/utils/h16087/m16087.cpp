#include "h16087/m16087.h"
QVector<double> m16087::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
