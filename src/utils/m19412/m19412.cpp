#include "m19412/m19412.h"
QVector<double> m19412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
