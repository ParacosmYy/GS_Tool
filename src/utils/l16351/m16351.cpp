#include "l16351/m16351.h"
QVector<double> m16351::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
