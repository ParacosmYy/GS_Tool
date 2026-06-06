#include "g16386/m16386.h"
QVector<double> m16386::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
