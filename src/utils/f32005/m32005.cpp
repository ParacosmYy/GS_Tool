#include "f32005/m32005.h"
QVector<double> m32005::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
