#include "l32011/m32011.h"
QVector<double> m32011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
