#include "a24200/m24200.h"
QVector<double> m24200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
