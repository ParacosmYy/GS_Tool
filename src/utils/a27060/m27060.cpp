#include "a27060/m27060.h"
QVector<double> m27060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
