#include "k17110/m17110.h"
QVector<double> m17110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
