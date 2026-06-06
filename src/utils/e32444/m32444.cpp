#include "e32444/m32444.h"
QVector<double> m32444::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
