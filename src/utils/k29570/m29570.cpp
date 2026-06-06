#include "k29570/m29570.h"
QVector<double> m29570::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
