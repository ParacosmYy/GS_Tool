#include "a18060/m18060.h"
QVector<double> m18060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
