#include "k9010/m9010.h"
QVector<double> m9010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
