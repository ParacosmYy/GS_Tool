#include "t25899/m25899.h"
QVector<double> m25899::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
