#include "e9904/m9904.h"
QVector<double> m9904::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
