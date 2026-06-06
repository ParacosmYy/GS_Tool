#include "a36060/m36060.h"
QVector<double> m36060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
