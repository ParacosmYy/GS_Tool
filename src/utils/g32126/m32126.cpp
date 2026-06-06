#include "g32126/m32126.h"
QVector<double> m32126::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
