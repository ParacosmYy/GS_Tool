#include "k16850/m16850.h"
QVector<double> m16850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
