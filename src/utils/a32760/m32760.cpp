#include "a32760/m32760.h"
QVector<double> m32760::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
