#include "f24225/m24225.h"
QVector<double> m24225::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
