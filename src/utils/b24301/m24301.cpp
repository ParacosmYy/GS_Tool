#include "b24301/m24301.h"
QVector<double> m24301::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
