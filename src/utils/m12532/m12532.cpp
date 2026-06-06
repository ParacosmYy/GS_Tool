#include "m12532/m12532.h"
QVector<double> m12532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
