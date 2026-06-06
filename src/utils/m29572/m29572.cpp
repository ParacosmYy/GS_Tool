#include "m29572/m29572.h"
QVector<double> m29572::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
