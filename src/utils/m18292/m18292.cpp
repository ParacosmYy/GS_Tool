#include "m18292/m18292.h"
QVector<double> m18292::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
