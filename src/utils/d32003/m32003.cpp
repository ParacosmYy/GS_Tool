#include "d32003/m32003.h"
QVector<double> m32003::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
