#include "h16287/m16287.h"
QVector<double> m16287::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
