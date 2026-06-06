#include "f8305/m8305.h"
QVector<double> m8305::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
