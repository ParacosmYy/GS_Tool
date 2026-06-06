#include "t9339/m9339.h"
QVector<double> m9339::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
