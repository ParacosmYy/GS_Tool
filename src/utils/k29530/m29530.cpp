#include "k29530/m29530.h"
QVector<double> m29530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
