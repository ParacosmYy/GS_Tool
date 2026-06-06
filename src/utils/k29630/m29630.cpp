#include "k29630/m29630.h"
QVector<double> m29630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
