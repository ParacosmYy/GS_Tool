#include "a32120/m32120.h"
QVector<double> m32120::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
