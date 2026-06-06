#include "a24600/m24600.h"
QVector<double> m24600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
