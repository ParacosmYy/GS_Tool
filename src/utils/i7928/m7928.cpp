#include "i7928/m7928.h"
QVector<double> m7928::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
