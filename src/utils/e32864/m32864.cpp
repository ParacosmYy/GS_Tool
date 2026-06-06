#include "e32864/m32864.h"
QVector<double> m32864::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
