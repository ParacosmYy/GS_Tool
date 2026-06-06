#include "d37583/m37583.h"
QVector<double> m37583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
