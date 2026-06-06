#include "m12752/m12752.h"
QVector<double> m12752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
