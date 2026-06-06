#include "e32564/m32564.h"
QVector<double> m32564::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
