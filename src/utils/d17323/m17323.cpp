#include "d17323/m17323.h"
QVector<double> m17323::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
