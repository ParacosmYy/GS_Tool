#include "d15223/m15223.h"
QVector<double> m15223::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
