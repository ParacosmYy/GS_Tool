#include "l8071/m8071.h"
QVector<double> m8071::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
