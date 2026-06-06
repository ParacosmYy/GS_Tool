#include "c16762/m16762.h"
QVector<double> m16762::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
