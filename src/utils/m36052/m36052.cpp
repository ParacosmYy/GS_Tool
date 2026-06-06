#include "m36052/m36052.h"
QVector<double> m36052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
