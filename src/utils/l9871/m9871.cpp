#include "l9871/m9871.h"
QVector<double> m9871::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
