#include "m16772/m16772.h"
QVector<double> m16772::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
