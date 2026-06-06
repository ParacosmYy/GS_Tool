#include "h15047/m15047.h"
QVector<double> m15047::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
