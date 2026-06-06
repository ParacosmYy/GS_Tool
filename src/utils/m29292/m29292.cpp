#include "m29292/m29292.h"
QVector<double> m29292::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
