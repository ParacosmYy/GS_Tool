#include "k15190/m15190.h"
QVector<double> m15190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
