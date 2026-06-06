#include "j32009/m32009.h"
QVector<double> m32009::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
