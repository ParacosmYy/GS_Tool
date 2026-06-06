#include "b18601/m18601.h"
QVector<double> m18601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
