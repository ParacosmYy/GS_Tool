#include "k37090/m37090.h"
QVector<double> m37090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
