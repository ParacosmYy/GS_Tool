#include "f8705/m8705.h"
QVector<double> m8705::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
