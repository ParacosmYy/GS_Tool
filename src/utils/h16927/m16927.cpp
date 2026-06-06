#include "h16927/m16927.h"
QVector<double> m16927::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
