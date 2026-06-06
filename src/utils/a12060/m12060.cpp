#include "a12060/m12060.h"
QVector<double> m12060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
