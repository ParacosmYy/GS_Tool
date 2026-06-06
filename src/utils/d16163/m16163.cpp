#include "d16163/m16163.h"
QVector<double> m16163::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
