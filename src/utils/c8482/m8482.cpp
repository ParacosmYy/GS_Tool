#include "c8482/m8482.h"
QVector<double> m8482::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
