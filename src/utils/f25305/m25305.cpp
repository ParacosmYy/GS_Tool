#include "f25305/m25305.h"
QVector<double> m25305::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
