#include "k13090/m13090.h"
QVector<double> m13090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
