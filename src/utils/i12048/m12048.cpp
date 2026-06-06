#include "i12048/m12048.h"
QVector<double> m12048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
