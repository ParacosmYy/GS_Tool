#include "k29170/m29170.h"
QVector<double> m29170::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
