#include "k25090/m25090.h"
QVector<double> m25090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
