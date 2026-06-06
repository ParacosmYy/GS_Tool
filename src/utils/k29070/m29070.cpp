#include "k29070/m29070.h"
QVector<double> m29070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
