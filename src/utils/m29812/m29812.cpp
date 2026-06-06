#include "m29812/m29812.h"
QVector<double> m29812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
