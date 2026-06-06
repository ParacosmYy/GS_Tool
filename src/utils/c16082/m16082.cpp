#include "c16082/m16082.h"
QVector<double> m16082::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
