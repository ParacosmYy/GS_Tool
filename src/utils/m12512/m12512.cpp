#include "m12512/m12512.h"
QVector<double> m12512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
