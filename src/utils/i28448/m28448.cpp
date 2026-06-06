#include "i28448/m28448.h"
QVector<double> m28448::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
