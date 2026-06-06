#include "k19110/m19110.h"
QVector<double> m19110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
