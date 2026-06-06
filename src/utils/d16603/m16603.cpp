#include "d16603/m16603.h"
QVector<double> m16603::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
