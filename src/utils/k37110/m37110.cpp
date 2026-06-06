#include "k37110/m37110.h"
QVector<double> m37110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
