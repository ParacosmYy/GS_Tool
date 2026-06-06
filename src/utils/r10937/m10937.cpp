#include "r10937/m10937.h"
QVector<double> m10937::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
