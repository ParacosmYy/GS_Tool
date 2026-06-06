#include "h25327/m25327.h"
QVector<double> m25327::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
