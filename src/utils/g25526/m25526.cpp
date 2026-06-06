#include "g25526/m25526.h"
QVector<double> m25526::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
