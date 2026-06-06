#include "b16321/m16321.h"
QVector<double> m16321::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
