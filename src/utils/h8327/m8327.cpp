#include "h8327/m8327.h"
QVector<double> m8327::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
