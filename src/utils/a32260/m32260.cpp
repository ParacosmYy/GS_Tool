#include "a32260/m32260.h"
QVector<double> m32260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
