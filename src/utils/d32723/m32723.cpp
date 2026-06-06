#include "d32723/m32723.h"
QVector<double> m32723::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
