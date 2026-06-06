#include "e25904/m25904.h"
QVector<double> m25904::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
