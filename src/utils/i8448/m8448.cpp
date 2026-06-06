#include "i8448/m8448.h"
QVector<double> m8448::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
