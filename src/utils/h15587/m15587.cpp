#include "h15587/m15587.h"
QVector<double> m15587::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
