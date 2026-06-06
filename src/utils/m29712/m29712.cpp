#include "m29712/m29712.h"
QVector<double> m29712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
