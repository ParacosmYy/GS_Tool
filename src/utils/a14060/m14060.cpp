#include "a14060/m14060.h"
QVector<double> m14060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
