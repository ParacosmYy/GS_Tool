#include "a32520/m32520.h"
QVector<double> m32520::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
