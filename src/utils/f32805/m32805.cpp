#include "f32805/m32805.h"
QVector<double> m32805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
