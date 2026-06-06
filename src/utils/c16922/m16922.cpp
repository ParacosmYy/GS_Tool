#include "c16922/m16922.h"
QVector<double> m16922::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
