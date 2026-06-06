#include "k29030/m29030.h"
QVector<double> m29030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
