#include "m13012/m13012.h"
QVector<double> m13012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
