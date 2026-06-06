#include "i20708/m20708.h"
QVector<double> m20708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
