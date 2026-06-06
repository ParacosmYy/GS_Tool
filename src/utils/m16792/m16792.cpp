#include "m16792/m16792.h"
QVector<double> m16792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
