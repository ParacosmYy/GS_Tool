#include "m12012/m12012.h"
QVector<double> m12012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
