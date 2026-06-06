#include "m32832/m32832.h"
QVector<double> m32832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
