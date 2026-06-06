#include "l7991/m7991.h"
QVector<double> m7991::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
