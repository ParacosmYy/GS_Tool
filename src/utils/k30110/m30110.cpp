#include "k30110/m30110.h"
QVector<double> m30110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
