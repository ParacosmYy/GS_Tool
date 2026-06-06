#include "m13412/m13412.h"
QVector<double> m13412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
