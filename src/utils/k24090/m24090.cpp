#include "k24090/m24090.h"
QVector<double> m24090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
