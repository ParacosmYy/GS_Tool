#include "b15301/m15301.h"
QVector<double> m15301::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
