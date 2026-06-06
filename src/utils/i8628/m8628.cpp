#include "i8628/m8628.h"
QVector<double> m8628::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
