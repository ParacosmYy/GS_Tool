#include "d10723/m10723.h"
QVector<double> m10723::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
