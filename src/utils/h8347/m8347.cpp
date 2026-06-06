#include "h8347/m8347.h"
QVector<double> m8347::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
