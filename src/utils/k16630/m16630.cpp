#include "k16630/m16630.h"
QVector<double> m16630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
