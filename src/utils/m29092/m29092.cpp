#include "m29092/m29092.h"
QVector<double> m29092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
