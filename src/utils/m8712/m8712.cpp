#include "m8712/m8712.h"
QVector<double> m8712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
