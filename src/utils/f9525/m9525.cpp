#include "f9525/m9525.h"
QVector<double> m9525::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
