#include "f9305/m9305.h"
QVector<double> m9305::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
