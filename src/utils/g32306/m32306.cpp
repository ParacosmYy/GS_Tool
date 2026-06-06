#include "g32306/m32306.h"
QVector<double> m32306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
