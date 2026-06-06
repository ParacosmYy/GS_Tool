#include "r32017/m32017.h"
QVector<double> m32017::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
