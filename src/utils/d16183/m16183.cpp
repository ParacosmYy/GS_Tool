#include "d16183/m16183.h"
QVector<double> m16183::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
