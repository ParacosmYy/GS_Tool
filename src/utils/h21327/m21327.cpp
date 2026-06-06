#include "h21327/m21327.h"
QVector<double> m21327::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
