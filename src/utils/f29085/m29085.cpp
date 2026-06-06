#include "f29085/m29085.h"
QVector<double> m29085::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
