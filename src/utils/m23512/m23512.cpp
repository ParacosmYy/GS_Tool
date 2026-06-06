#include "m23512/m23512.h"
QVector<double> m23512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
