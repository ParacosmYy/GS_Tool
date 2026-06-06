#include "f18485/m18485.h"
QVector<double> m18485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
