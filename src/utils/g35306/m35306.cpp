#include "g35306/m35306.h"
QVector<double> m35306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
