#include "k17910/m17910.h"
QVector<double> m17910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
