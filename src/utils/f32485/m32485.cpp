#include "f32485/m32485.h"
QVector<double> m32485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
