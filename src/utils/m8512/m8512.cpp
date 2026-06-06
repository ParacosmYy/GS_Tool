#include "m8512/m8512.h"
QVector<double> m8512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
