#include "b16021/m16021.h"
QVector<double> m16021::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
