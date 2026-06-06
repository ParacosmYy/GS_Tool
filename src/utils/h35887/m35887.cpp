#include "h35887/m35887.h"
QVector<double> m35887::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
