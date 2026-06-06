#include "g25626/m25626.h"
QVector<double> m25626::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
