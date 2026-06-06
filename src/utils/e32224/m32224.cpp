#include "e32224/m32224.h"
QVector<double> m32224::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
