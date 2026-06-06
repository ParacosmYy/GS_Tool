#include "m19072/m19072.h"
QVector<double> m19072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
