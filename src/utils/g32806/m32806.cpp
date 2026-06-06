#include "g32806/m32806.h"
QVector<double> m32806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
