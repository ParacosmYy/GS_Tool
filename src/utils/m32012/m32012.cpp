#include "m32012/m32012.h"
QVector<double> m32012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
