#include "c16582/m16582.h"
QVector<double> m16582::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
