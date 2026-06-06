#include "e32024/m32024.h"
QVector<double> m32024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
