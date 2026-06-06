#include "k31530/m31530.h"
QVector<double> m31530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
