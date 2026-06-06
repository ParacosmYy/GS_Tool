#include "a15360/m15360.h"
QVector<double> m15360::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
