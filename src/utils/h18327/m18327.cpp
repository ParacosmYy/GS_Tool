#include "h18327/m18327.h"
QVector<double> m18327::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
