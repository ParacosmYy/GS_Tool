#include "m17052/m17052.h"
QVector<double> m17052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
