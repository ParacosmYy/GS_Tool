#include "n7933/m7933.h"
QVector<double> m7933::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
