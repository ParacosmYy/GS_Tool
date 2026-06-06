#include "b8081/m8081.h"
QVector<double> m8081::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
