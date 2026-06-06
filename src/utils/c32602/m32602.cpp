#include "c32602/m32602.h"
QVector<double> m32602::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
