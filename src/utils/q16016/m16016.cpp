#include "q16016/m16016.h"
QVector<double> m16016::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
