#include "g25306/m25306.h"
QVector<double> m25306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
