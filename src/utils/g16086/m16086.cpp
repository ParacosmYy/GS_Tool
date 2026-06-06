#include "g16086/m16086.h"
QVector<double> m16086::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
