#include "k29390/m29390.h"
QVector<double> m29390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
