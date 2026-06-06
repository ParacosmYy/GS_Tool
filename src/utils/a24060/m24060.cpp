#include "a24060/m24060.h"
QVector<double> m24060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
