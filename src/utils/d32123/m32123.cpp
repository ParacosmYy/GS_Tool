#include "d32123/m32123.h"
QVector<double> m32123::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
