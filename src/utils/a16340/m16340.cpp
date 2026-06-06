#include "a16340/m16340.h"
QVector<double> m16340::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
