#include "a24320/m24320.h"
QVector<double> m24320::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
