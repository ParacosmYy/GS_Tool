#include "m26412/m26412.h"
QVector<double> m26412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
