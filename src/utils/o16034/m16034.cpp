#include "o16034/m16034.h"
QVector<double> m16034::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
