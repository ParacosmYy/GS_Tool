#include "g16566/m16566.h"
QVector<double> m16566::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
