#include "m32532/m32532.h"
QVector<double> m32532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
