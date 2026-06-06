#include "e8904/m8904.h"
QVector<double> m8904::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
