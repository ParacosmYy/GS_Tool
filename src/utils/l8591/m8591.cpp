#include "l8591/m8591.h"
QVector<double> m8591::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
