#include "g25866/m25866.h"
QVector<double> m25866::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
