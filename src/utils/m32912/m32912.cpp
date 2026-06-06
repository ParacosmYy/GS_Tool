#include "m32912/m32912.h"
QVector<double> m32912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
