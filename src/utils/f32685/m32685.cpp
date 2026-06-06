#include "f32685/m32685.h"
QVector<double> m32685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
