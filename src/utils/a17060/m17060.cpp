#include "a17060/m17060.h"
QVector<double> m17060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
