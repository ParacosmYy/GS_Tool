#include "m32032/m32032.h"
QVector<double> m32032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
