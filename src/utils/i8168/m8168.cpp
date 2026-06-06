#include "i8168/m8168.h"
QVector<double> m8168::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
