#include "a9060/m9060.h"
QVector<double> m9060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
