#include "m24652/m24652.h"
QVector<double> m24652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
