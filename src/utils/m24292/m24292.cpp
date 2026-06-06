#include "m24292/m24292.h"
QVector<double> m24292::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
