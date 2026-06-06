#include "p16875/m16875.h"
QVector<double> m16875::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
