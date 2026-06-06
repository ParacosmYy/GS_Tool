#include "g32026/m32026.h"
QVector<double> m32026::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
