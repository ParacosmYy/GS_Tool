#include "a29980/m29980.h"
QVector<double> m29980::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
