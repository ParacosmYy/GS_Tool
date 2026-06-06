#include "k17070/m17070.h"
QVector<double> m17070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
