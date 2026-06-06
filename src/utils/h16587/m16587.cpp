#include "h16587/m16587.h"
QVector<double> m16587::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
