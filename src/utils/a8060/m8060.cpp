#include "a8060/m8060.h"
QVector<double> m8060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
