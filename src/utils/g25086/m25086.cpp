#include "g25086/m25086.h"
QVector<double> m25086::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
