#include "j8329/m8329.h"
QVector<double> m8329::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
