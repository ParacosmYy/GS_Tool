#include "l32911/m32911.h"
QVector<double> m32911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
