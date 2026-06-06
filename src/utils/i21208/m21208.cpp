#include "i21208/m21208.h"
QVector<double> m21208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
