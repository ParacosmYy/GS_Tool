#include "p8395/m8395.h"
QVector<double> m8395::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
