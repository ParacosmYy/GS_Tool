#include "j32749/m32749.h"
QVector<double> m32749::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
