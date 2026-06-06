#include "f32565/m32565.h"
QVector<double> m32565::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
