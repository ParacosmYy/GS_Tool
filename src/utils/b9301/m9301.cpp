#include "b9301/m9301.h"
QVector<double> m9301::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
