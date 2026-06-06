#include "m37632/m37632.h"
QVector<double> m37632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
