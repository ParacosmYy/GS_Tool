#include "i36328/m36328.h"
QVector<double> m36328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
