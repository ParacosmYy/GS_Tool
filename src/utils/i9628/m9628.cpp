#include "i9628/m9628.h"
QVector<double> m9628::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
