#include "b16401/m16401.h"
QVector<double> m16401::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
