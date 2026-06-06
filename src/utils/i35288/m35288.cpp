#include "i35288/m35288.h"
QVector<double> m35288::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
