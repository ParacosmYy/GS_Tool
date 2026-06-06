#include "i32768/m32768.h"
QVector<double> m32768::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
