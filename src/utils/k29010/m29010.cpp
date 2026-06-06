#include "k29010/m29010.h"
QVector<double> m29010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
