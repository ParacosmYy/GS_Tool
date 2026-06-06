#include "a15060/m15060.h"
QVector<double> m15060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
