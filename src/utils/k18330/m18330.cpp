#include "k18330/m18330.h"
QVector<double> m18330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
