#include "e16024/m16024.h"
QVector<double> m16024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
