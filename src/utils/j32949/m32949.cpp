#include "j32949/m32949.h"
QVector<double> m32949::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
