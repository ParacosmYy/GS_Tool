#include "a22060/m22060.h"
QVector<double> m22060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
