#include "e32004/m32004.h"
QVector<double> m32004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
