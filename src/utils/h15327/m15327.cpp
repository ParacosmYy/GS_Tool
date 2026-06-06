#include "h15327/m15327.h"
QVector<double> m15327::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
