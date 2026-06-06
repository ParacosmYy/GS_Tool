#include "g16626/m16626.h"
QVector<double> m16626::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
