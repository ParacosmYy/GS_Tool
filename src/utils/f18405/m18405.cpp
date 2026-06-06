#include "f18405/m18405.h"
QVector<double> m18405::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
