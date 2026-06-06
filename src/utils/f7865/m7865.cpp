#include "f7865/m7865.h"
QVector<double> m7865::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
