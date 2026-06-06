#include "f18305/m18305.h"
QVector<double> m18305::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
