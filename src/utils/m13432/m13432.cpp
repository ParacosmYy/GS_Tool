#include "m13432/m13432.h"
QVector<double> m13432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
