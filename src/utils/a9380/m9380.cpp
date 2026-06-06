#include "a9380/m9380.h"
QVector<double> m9380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
