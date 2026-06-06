#include "m12872/m12872.h"
QVector<double> m12872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
