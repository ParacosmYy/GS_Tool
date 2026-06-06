#include "j32809/m32809.h"
QVector<double> m32809::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
