#include "m16372/m16372.h"
QVector<double> m16372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
