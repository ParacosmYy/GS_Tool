#include "l16251/m16251.h"
QVector<double> m16251::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
