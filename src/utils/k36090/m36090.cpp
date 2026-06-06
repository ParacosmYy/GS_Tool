#include "k36090/m36090.h"
QVector<double> m36090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
