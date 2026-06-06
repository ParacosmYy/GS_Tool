#include "g8306/m8306.h"
QVector<double> m8306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
