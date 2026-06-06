#include "a26060/m26060.h"
QVector<double> m26060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
