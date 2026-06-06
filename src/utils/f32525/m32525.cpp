#include "f32525/m32525.h"
QVector<double> m32525::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
