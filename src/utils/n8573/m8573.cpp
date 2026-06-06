#include "n8573/m8573.h"
QVector<double> m8573::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
