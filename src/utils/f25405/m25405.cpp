#include "f25405/m25405.h"
QVector<double> m25405::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
