#include "m24712/m24712.h"
QVector<double> m24712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
