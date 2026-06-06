#include "d32023/m32023.h"
QVector<double> m32023::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
