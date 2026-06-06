#include "h16227/m16227.h"
QVector<double> m16227::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
