#include "c16222/m16222.h"
QVector<double> m16222::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
