#include "o8534/m8534.h"
QVector<double> m8534::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
