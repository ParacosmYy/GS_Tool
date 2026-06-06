#include "m24012/m24012.h"
QVector<double> m24012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
