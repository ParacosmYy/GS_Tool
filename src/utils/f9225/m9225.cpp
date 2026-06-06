#include "f9225/m9225.h"
QVector<double> m9225::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
