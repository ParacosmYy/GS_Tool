#include "f9405/m9405.h"
QVector<double> m9405::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
