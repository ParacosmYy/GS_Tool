#include "k14910/m14910.h"
QVector<double> m14910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
