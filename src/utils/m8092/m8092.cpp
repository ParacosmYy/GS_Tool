#include "m8092/m8092.h"
QVector<double> m8092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
