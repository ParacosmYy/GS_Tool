#include "e16724/m16724.h"
QVector<double> m16724::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
