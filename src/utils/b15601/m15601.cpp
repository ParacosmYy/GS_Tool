#include "b15601/m15601.h"
QVector<double> m15601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
