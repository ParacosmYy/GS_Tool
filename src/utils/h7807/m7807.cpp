#include "h7807/m7807.h"
QVector<double> m7807::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
