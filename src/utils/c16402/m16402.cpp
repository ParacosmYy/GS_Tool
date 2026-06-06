#include "c16402/m16402.h"
QVector<double> m16402::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
