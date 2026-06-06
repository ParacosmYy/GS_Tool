#include "a16060/m16060.h"
QVector<double> m16060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
