#include "f24405/m24405.h"
QVector<double> m24405::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
