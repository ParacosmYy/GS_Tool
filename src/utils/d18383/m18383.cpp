#include "d18383/m18383.h"
QVector<double> m18383::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
