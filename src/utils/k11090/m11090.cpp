#include "k11090/m11090.h"
QVector<double> m11090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
