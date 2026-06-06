#include "i25868/m25868.h"
QVector<double> m25868::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
