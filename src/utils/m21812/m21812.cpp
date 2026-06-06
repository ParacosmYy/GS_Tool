#include "m21812/m21812.h"
QVector<double> m21812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
